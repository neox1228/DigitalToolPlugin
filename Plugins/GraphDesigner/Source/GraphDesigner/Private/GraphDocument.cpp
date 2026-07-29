#include "GraphDocument.h"

#include "GraphDesigner.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

UGraphDocument::UGraphDocument()
{
	DocumentId = MakeId(TEXT("Document"));
}

void UGraphDocument::Clear()
{
	Elements.Empty();
	Connections.Empty();
	RequiredConnections.Empty();
	LastError.Reset();
}

FString UGraphDocument::AddElement(EGraphDesignerElementType Type, FVector2D Position, FVector2D Size)
{
	const FString ElementId = MakeId(TEXT("Element"));
	Elements.Add(FGraphDesignerElement(ElementId, Type, Position, Size));
	return ElementId;
}

bool UGraphDocument::RemoveElement(const FString& ElementId)
{
	const int32 RemovedCount = Elements.RemoveAll([&ElementId](const FGraphDesignerElement& Element)
	{
		return Element.Id == ElementId;
	});

	if (RemovedCount <= 0)
	{
		return false;
	}

	Connections.RemoveAll([this](const FGraphDesignerConnection& Connection)
	{
		return !PinExists(Connection.StartPinId) || !PinExists(Connection.EndPinId);
	});

	RequiredConnections.RemoveAll([this](const FGraphDesignerRequiredConnection& RequiredConnection)
	{
		return !PinExists(RequiredConnection.StartPinId) || !PinExists(RequiredConnection.EndPinId);
	});

	return true;
}

int32 UGraphDocument::RemoveElements(const TArray<FString>& ElementIds)
{
	int32 RemovedCount = 0;
	for (const FString& ElementId : ElementIds)
	{
		RemovedCount += RemoveElement(ElementId) ? 1 : 0;
	}
	return RemovedCount;
}

bool UGraphDocument::UpdateElement(const FGraphDesignerElement& Element)
{
	if (FGraphDesignerElement* ExistingElement = FindElement(Element.Id))
	{
		*ExistingElement = Element;
		for (FGraphDesignerPin& Pin : ExistingElement->Pins)
		{
			Pin.OwnerElementId = ExistingElement->Id;
		}
		RefreshElementPinIds(ExistingElement->Id);
		return true;
	}
	return false;
}

bool UGraphDocument::SetElementSelected(const FString& ElementId, bool bSelected)
{
	if (FGraphDesignerElement* Element = FindElement(ElementId))
	{
		Element->bSelected = bSelected;
		return true;
	}
	return false;
}

void UGraphDocument::ClearSelection()
{
	for (FGraphDesignerElement& Element : Elements)
	{
		Element.bSelected = false;
		Element.bDragging = false;
		for (FGraphDesignerPin& Pin : Element.Pins)
		{
			Pin.bSelected = false;
		}
	}

	for (FGraphDesignerConnection& Connection : Connections)
	{
		Connection.bSelected = false;
	}
}

FString UGraphDocument::AddPinToElement(const FString& ElementId, FVector2D RelativePosition, const FString& PinText)
{
	FGraphDesignerElement* Element = FindElement(ElementId);
	if (Element == nullptr)
	{
		LastError = FString::Printf(TEXT("Element '%s' was not found."), *ElementId);
		return TEXT("");
	}

	FGraphDesignerPin NewPin(TEXT(""), ElementId, RelativePosition);
	NewPin.Text = PinText;
	NewPin.Id = MakeUniquePinId(MakePinId(*Element, NewPin));
	Element->Pins.Add(NewPin);
	return NewPin.Id;
}

bool UGraphDocument::RemovePin(const FString& PinId)
{
	for (FGraphDesignerElement& Element : Elements)
	{
		const int32 RemovedCount = Element.Pins.RemoveAll([&PinId](const FGraphDesignerPin& Pin)
		{
			return Pin.Id == PinId;
		});

		if (RemovedCount > 0)
		{
			Connections.RemoveAll([&PinId](const FGraphDesignerConnection& Connection)
			{
				return Connection.StartPinId == PinId || Connection.EndPinId == PinId;
			});
			RequiredConnections.RemoveAll([&PinId](const FGraphDesignerRequiredConnection& RequiredConnection)
			{
				return RequiredConnection.StartPinId == PinId || RequiredConnection.EndPinId == PinId;
			});
			return true;
		}
	}

	return false;
}

bool UGraphDocument::UpdatePin(const FGraphDesignerPin& Pin)
{
	FGraphDesignerElement* OwnerElement = nullptr;
	if (FGraphDesignerPin* ExistingPin = FindPin(Pin.Id, &OwnerElement))
	{
		*ExistingPin = Pin;
		if (OwnerElement != nullptr)
		{
			ExistingPin->OwnerElementId = OwnerElement->Id;
			RefreshPinId(*OwnerElement, *ExistingPin);
		}
		return true;
	}
	return false;
}

bool UGraphDocument::ArrangePinsOnSameSide(const FString& ElementId, FVector2D SidePosition)
{
	FGraphDesignerElement* Element = FindElement(ElementId);
	if (Element == nullptr)
	{
		return false;
	}

	enum class EPinSide
	{
		Left,
		Right,
		Top,
		Bottom
	};

	EPinSide Side = EPinSide::Left;
	const float LeftDistance = FMath::Abs(SidePosition.X);
	const float RightDistance = FMath::Abs(SidePosition.X - Element->Size.X);
	const float TopDistance = FMath::Abs(SidePosition.Y);
	const float BottomDistance = FMath::Abs(SidePosition.Y - Element->Size.Y);
	float BestDistance = LeftDistance;

	if (RightDistance < BestDistance)
	{
		BestDistance = RightDistance;
		Side = EPinSide::Right;
	}
	if (TopDistance < BestDistance)
	{
		BestDistance = TopDistance;
		Side = EPinSide::Top;
	}
	if (BottomDistance < BestDistance)
	{
		Side = EPinSide::Bottom;
	}

	TArray<FGraphDesignerPin*> SidePins;
	for (FGraphDesignerPin& Pin : Element->Pins)
	{
		const bool bLeft = FMath::IsNearlyZero(Pin.RelativePosition.X);
		const bool bRight = FMath::IsNearlyEqual(Pin.RelativePosition.X, Element->Size.X);
		const bool bTop = FMath::IsNearlyZero(Pin.RelativePosition.Y);
		const bool bBottom = FMath::IsNearlyEqual(Pin.RelativePosition.Y, Element->Size.Y);

		if ((Side == EPinSide::Left && bLeft)
			|| (Side == EPinSide::Right && bRight)
			|| (Side == EPinSide::Top && bTop)
			|| (Side == EPinSide::Bottom && bBottom))
		{
			SidePins.Add(&Pin);
		}
	}

	SidePins.Sort([Side](const FGraphDesignerPin& A, const FGraphDesignerPin& B)
	{
		return (Side == EPinSide::Left || Side == EPinSide::Right)
			? A.RelativePosition.Y < B.RelativePosition.Y
			: A.RelativePosition.X < B.RelativePosition.X;
	});

	for (int32 Index = 0; Index < SidePins.Num(); ++Index)
	{
		const float Ratio = static_cast<float>(Index + 1) / static_cast<float>(SidePins.Num() + 1);
		switch (Side)
		{
		case EPinSide::Left:
			SidePins[Index]->RelativePosition = FVector2D(0.0f, Element->Size.Y * Ratio);
			break;
		case EPinSide::Right:
			SidePins[Index]->RelativePosition = FVector2D(Element->Size.X, Element->Size.Y * Ratio);
			break;
		case EPinSide::Top:
			SidePins[Index]->RelativePosition = FVector2D(Element->Size.X * Ratio, 0.0f);
			break;
		case EPinSide::Bottom:
			SidePins[Index]->RelativePosition = FVector2D(Element->Size.X * Ratio, Element->Size.Y);
			break;
		default:
			break;
		}
	}

	return true;
}

FString UGraphDocument::AddConnection(const FString& StartPinId, const FString& EndPinId)
{
	return AddConnectionWithRoutePoints(StartPinId, EndPinId, TArray<FVector2D>());
}

FString UGraphDocument::AddConnectionWithRoutePoints(const FString& StartPinId, const FString& EndPinId, const TArray<FVector2D>& RoutePoints)
{
	if (StartPinId.IsEmpty() || EndPinId.IsEmpty() || StartPinId == EndPinId)
	{
		LastError = TEXT("Connection requires two different pins.");
		return TEXT("");
	}

	const FGraphDesignerElement* StartOwnerElement = nullptr;
	const FGraphDesignerElement* EndOwnerElement = nullptr;
	const FGraphDesignerPin* StartPin = FindPin(StartPinId, &StartOwnerElement);
	const FGraphDesignerPin* EndPin = FindPin(EndPinId, &EndOwnerElement);
	if (StartPin == nullptr || EndPin == nullptr)
	{
		LastError = TEXT("Connection pin was not found.");
		return TEXT("");
	}

	if (StartOwnerElement != nullptr && EndOwnerElement != nullptr && StartOwnerElement->Id == EndOwnerElement->Id)
	{
		LastError = TEXT("Pins on the same element cannot be connected.");
		return TEXT("");
	}

	const bool bStartAlreadyConnected = Connections.ContainsByPredicate([&StartPinId](const FGraphDesignerConnection& Connection)
	{
		return Connection.StartPinId == StartPinId || Connection.EndPinId == StartPinId;
	});
	if (bStartAlreadyConnected)
	{
		LastError = TEXT("Start pin is already connected.");
		return TEXT("");
	}

	const bool bEndAlreadyConnected = Connections.ContainsByPredicate([&EndPinId](const FGraphDesignerConnection& Connection)
	{
		return Connection.StartPinId == EndPinId || Connection.EndPinId == EndPinId;
	});
	if (bEndAlreadyConnected)
	{
		LastError = TEXT("End pin is already connected.");
		return TEXT("");
	}

	const bool bAlreadyConnected = Connections.ContainsByPredicate([&StartPinId, &EndPinId](const FGraphDesignerConnection& Connection)
	{
		return (Connection.StartPinId == StartPinId && Connection.EndPinId == EndPinId)
			|| (Connection.StartPinId == EndPinId && Connection.EndPinId == StartPinId);
	});

	if (bAlreadyConnected)
	{
		LastError = TEXT("Pins are already connected.");
		return TEXT("");
	}

	const FString ConnectionId = MakeId(TEXT("Connection"));
	FGraphDesignerConnection NewConnection(ConnectionId, StartPinId, EndPinId);
	NewConnection.RoutePoints = RoutePoints;
	Connections.Add(NewConnection);
	LastError.Reset();
	return ConnectionId;
}

bool UGraphDocument::RemoveConnection(const FString& ConnectionId)
{
	return Connections.RemoveAll([&ConnectionId](const FGraphDesignerConnection& Connection)
	{
		return Connection.Id == ConnectionId;
	}) > 0;
}

bool UGraphDocument::RemoveConnectionBetweenPins(const FString& StartPinId, const FString& EndPinId)
{
	if (StartPinId.IsEmpty() || EndPinId.IsEmpty() || StartPinId == EndPinId)
	{
		LastError = TEXT("Connection removal requires two different pins.");
		return false;
	}

	const int32 RemovedCount = Connections.RemoveAll([&StartPinId, &EndPinId](const FGraphDesignerConnection& Connection)
	{
		return (Connection.StartPinId == StartPinId && Connection.EndPinId == EndPinId)
			|| (Connection.StartPinId == EndPinId && Connection.EndPinId == StartPinId);
	});

	if (RemovedCount <= 0)
	{
		LastError = TEXT("Connection was not found.");
		return false;
	}

	LastError.Reset();
	return true;
}

bool UGraphDocument::UpdateConnection(const FGraphDesignerConnection& Connection)
{
	if (FGraphDesignerConnection* ExistingConnection = FindConnection(Connection.Id))
	{
		*ExistingConnection = Connection;
		return true;
	}
	return false;
}

bool UGraphDocument::SetConnectionLineStyle(const FString& ConnectionId, EGraphDesignerConnectionLineStyle LineStyle)
{
	if (FGraphDesignerConnection* Connection = FindConnection(ConnectionId))
	{
		Connection->LineStyle = LineStyle;
		return true;
	}
	return false;
}

void UGraphDocument::SetRequiredConnections(const TArray<FGraphDesignerRequiredConnection>& InRequiredConnections)
{
	RequiredConnections = InRequiredConnections;
}

TArray<FGraphDesignerRequiredConnection> UGraphDocument::GetRequiredConnections() const
{
	return RequiredConnections;
}

int32 UGraphDocument::AddRequiredConnection(const FString& StartPinId, const FString& EndPinId)
{
	RequiredConnections.Add(FGraphDesignerRequiredConnection(StartPinId, EndPinId));
	return RequiredConnections.Num() - 1;
}

bool UGraphDocument::UpdateRequiredConnection(int32 Index, const FString& StartPinId, const FString& EndPinId)
{
	if (!RequiredConnections.IsValidIndex(Index))
	{
		return false;
	}

	RequiredConnections[Index].StartPinId = StartPinId;
	RequiredConnections[Index].EndPinId = EndPinId;
	return true;
}

bool UGraphDocument::RemoveRequiredConnectionAt(int32 Index)
{
	if (!RequiredConnections.IsValidIndex(Index))
	{
		return false;
	}

	RequiredConnections.RemoveAt(Index);
	return true;
}

bool UGraphDocument::IsRequiredConnection(const FString& StartPinId, const FString& EndPinId) const
{
	return RequiredConnections.ContainsByPredicate([&StartPinId, &EndPinId](const FGraphDesignerRequiredConnection& RequiredConnection)
	{
		return RequiredConnection.StartPinId == StartPinId && RequiredConnection.EndPinId == EndPinId;
	});
}

bool UGraphDocument::IsRequiredConnectionUndirected(const FString& StartPinId, const FString& EndPinId, FString& AnswerStartPinId, FString& AnswerEndPinId) const
{
	return RequiredConnections.ContainsByPredicate([&StartPinId, &EndPinId, &AnswerStartPinId, &AnswerEndPinId](const FGraphDesignerRequiredConnection& RequiredConnection)
	{
		bool bConnectionTrue = (RequiredConnection.StartPinId == StartPinId && RequiredConnection.EndPinId == EndPinId) || (RequiredConnection.StartPinId == EndPinId && RequiredConnection.EndPinId == StartPinId);
		if (bConnectionTrue)
		{
			AnswerStartPinId = RequiredConnection.StartPinId;
			AnswerEndPinId = RequiredConnection.EndPinId;
		}
		return bConnectionTrue;
	});
}

TArray<FGraphDesignerElement> UGraphDocument::GetElements() const
{
	return Elements;
}

TArray<FGraphDesignerConnection> UGraphDocument::GetConnections() const
{
	return Connections;
}

bool UGraphDocument::SaveToJsonString(FString& OutJson) const
{
	FGraphDesignerDocumentData Data;
	Data.DocumentId = DocumentId;
	Data.Elements = Elements;
	for (FGraphDesignerElement& Element : Data.Elements)
	{
		if (Element.Type == EGraphDesignerElementType::Image && !Element.ImagePath.IsEmpty())
		{
			Element.ImagePath = MakeRelativeImagePath(Element.ImagePath);
		}
	}
	Data.Connections = Connections;
	Data.RequiredConnections = RequiredConnections;

	if (!FJsonObjectConverter::UStructToJsonObjectString(Data, OutJson))
	{
		UE_LOG(LogGraphDesigner, Warning, TEXT("Failed to serialize graph document."));
		return false;
	}

	return true;
}

bool UGraphDocument::LoadFromJsonString(const FString& Json)
{
	FGraphDesignerDocumentData Data;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(Json, &Data, 0, 0))
	{
		LastError = TEXT("Failed to parse graph JSON.");
		return false;
	}

	DocumentId = Data.DocumentId.IsEmpty() ? MakeId(TEXT("Document")) : Data.DocumentId;
	Elements = Data.Elements;
	for (FGraphDesignerElement& Element : Elements)
	{
		if (Element.Type == EGraphDesignerElementType::Image && !Element.ImagePath.IsEmpty())
		{
			Element.ImagePath = ResolveImagePath(Element.ImagePath);
		}
	}
	Connections = Data.Connections;
	RequiredConnections = Data.RequiredConnections;
	RefreshAllPinIds();
	LastError.Reset();
	return true;
}

bool UGraphDocument::SaveToFile(const FString& GraphName) const
{
	FString Json;
	if (!SaveToJsonString(Json))
	{
		return false;
	}

	const FString Directory = GetDefaultGraphDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory) && !PlatformFile.CreateDirectoryTree(*Directory))
	{
		UE_LOG(LogGraphDesigner, Warning, TEXT("Failed to create graph directory: %s"), *Directory);
		return false;
	}

	const FString FilePath = FPaths::Combine(Directory, NormalizeGraphFileName(GraphName));
	return FFileHelper::SaveStringToFile(Json, *FilePath);
}

bool UGraphDocument::LoadFromFile(const FString& GraphName)
{
	const FString FilePath = FPaths::Combine(GetDefaultGraphDirectory(), NormalizeGraphFileName(GraphName));

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *FilePath))
	{
		LastError = FString::Printf(TEXT("Failed to load graph file '%s'."), *FilePath);
		return false;
	}

	return LoadFromJsonString(Json);
}

FString UGraphDocument::GetDefaultGraphDirectory()
{
	return FPaths::Combine(FPaths::ProjectContentDir(),TEXT("HuaYang"), TEXT("Graphs"));
}

FString UGraphDocument::MakeRelativeImagePath(const FString& Path)
{
	if (Path.IsEmpty())
	{
		return Path;
	}

	if (FPaths::IsRelative(Path))
	{
		FString NormalizedPath = Path;
		FPaths::NormalizeFilename(NormalizedPath);
		return NormalizedPath;
	}

	const FString AbsolutePath = FPaths::ConvertRelativePathToFull(Path);
	FString RelativePath = AbsolutePath;
	if (FPaths::MakePathRelativeTo(RelativePath, *FPaths::ProjectContentDir()))
	{
		FPaths::NormalizeFilename(RelativePath);
		return RelativePath;
	}

	return Path;
}

FString UGraphDocument::ResolveImagePath(const FString& StoredPath)
{
	if (StoredPath.IsEmpty())
	{
		return StoredPath;
	}

	if (FPaths::IsRelative(StoredPath))
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), StoredPath));
	}

	return FPaths::ConvertRelativePathToFull(StoredPath);
}

FGraphDesignerElement* UGraphDocument::FindElement(const FString& ElementId)
{
	return Elements.FindByPredicate([&ElementId](const FGraphDesignerElement& Element)
	{
		return Element.Id == ElementId;
	});
}

const FGraphDesignerElement* UGraphDocument::FindElement(const FString& ElementId) const
{
	return Elements.FindByPredicate([&ElementId](const FGraphDesignerElement& Element)
	{
		return Element.Id == ElementId;
	});
}

FGraphDesignerConnection* UGraphDocument::FindConnection(const FString& ConnectionId)
{
	return Connections.FindByPredicate([&ConnectionId](const FGraphDesignerConnection& Connection)
	{
		return Connection.Id == ConnectionId;
	});
}

const FGraphDesignerConnection* UGraphDocument::FindConnection(const FString& ConnectionId) const
{
	return Connections.FindByPredicate([&ConnectionId](const FGraphDesignerConnection& Connection)
	{
		return Connection.Id == ConnectionId;
	});
}

FGraphDesignerPin* UGraphDocument::FindPin(const FString& PinId, FGraphDesignerElement** OutOwnerElement)
{
	for (FGraphDesignerElement& Element : Elements)
	{
		for (FGraphDesignerPin& Pin : Element.Pins)
		{
			if (Pin.Id == PinId)
			{
				if (OutOwnerElement != nullptr)
				{
					*OutOwnerElement = &Element;
				}
				return &Pin;
			}
		}
	}

	return nullptr;
}

const FGraphDesignerPin* UGraphDocument::FindPin(const FString& PinId, const FGraphDesignerElement** OutOwnerElement) const
{
	for (const FGraphDesignerElement& Element : Elements)
	{
		for (const FGraphDesignerPin& Pin : Element.Pins)
		{
			if (Pin.Id == PinId)
			{
				if (OutOwnerElement != nullptr)
				{
					*OutOwnerElement = &Element;
				}
				return &Pin;
			}
		}
	}

	return nullptr;
}

bool UGraphDocument::GetPinWorldPosition(const FString& PinId, FVector2D& OutWorldPosition) const
{
	const FGraphDesignerElement* OwnerElement = nullptr;
	const FGraphDesignerPin* Pin = FindPin(PinId, &OwnerElement);
	if (Pin == nullptr || OwnerElement == nullptr)
	{
		return false;
	}

	OutWorldPosition = OwnerElement->Position + Pin->RelativePosition;
	return true;
}

void UGraphDocument::RefreshElementPinIds(const FString& ElementId)
{
	FGraphDesignerElement* Element = FindElement(ElementId);
	if (Element == nullptr)
	{
		return;
	}

	for (FGraphDesignerPin& Pin : Element->Pins)
	{
		RefreshPinId(*Element, Pin);
	}
}

FString UGraphDocument::MakeId(const TCHAR* Prefix)
{
	return FString::Printf(TEXT("%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
}

FString UGraphDocument::NormalizeGraphFileName(const FString& GraphName)
{
	FString FileName = GraphName.IsEmpty() ? TEXT("Graph") : GraphName;
	FileName = FPaths::GetCleanFilename(FileName);

	if (FPaths::GetExtension(FileName).IsEmpty())
	{
		FileName += TEXT(".json");
	}

	return FileName;
}

FString UGraphDocument::NormalizePinIdPart(const FString& Text, const FString& Fallback)
{
	FString Part = Text.TrimStartAndEnd();
	if (Part.IsEmpty())
	{
		Part = Fallback.TrimStartAndEnd();
	}
	if (Part.IsEmpty())
	{
		Part = TEXT("Pin");
	}

	Part.ReplaceInline(TEXT(" "), TEXT("_"));
	Part.ReplaceInline(TEXT("\t"), TEXT("_"));
	Part.ReplaceInline(TEXT("\r"), TEXT("_"));
	Part.ReplaceInline(TEXT("\n"), TEXT("_"));
	return Part;
}

FString UGraphDocument::MakePinId(const FGraphDesignerElement& OwnerElement, const FGraphDesignerPin& Pin) const
{
	const FString OwnerPart = NormalizePinIdPart(OwnerElement.Text, OwnerElement.Id);
	const FString PinPart = NormalizePinIdPart(Pin.Text, TEXT("Pin"));
	return FString::Printf(TEXT("%s_%s"), *OwnerPart, *PinPart);
}

FString UGraphDocument::MakeUniquePinId(const FString& BasePinId, const FString& CurrentPinId) const
{
	FString Candidate = BasePinId;
	int32 Suffix = 2;
	while (const FGraphDesignerPin* ExistingPin = FindPin(Candidate))
	{
		if (ExistingPin->Id == CurrentPinId)
		{
			break;
		}
		Candidate = FString::Printf(TEXT("%s_%d"), *BasePinId, Suffix++);
	}
	return Candidate;
}

void UGraphDocument::RefreshPinId(FGraphDesignerElement& OwnerElement, FGraphDesignerPin& Pin)
{
	const FString OldPinId = Pin.Id;
	const FString NewPinId = MakeUniquePinId(MakePinId(OwnerElement, Pin), OldPinId);
	if (OldPinId == NewPinId)
	{
		return;
	}

	Pin.Id = NewPinId;
	ReplacePinReferences(OldPinId, NewPinId);
}

void UGraphDocument::RefreshAllPinIds()
{
	for (FGraphDesignerElement& Element : Elements)
	{
		for (FGraphDesignerPin& Pin : Element.Pins)
		{
			Pin.OwnerElementId = Element.Id;
			RefreshPinId(Element, Pin);
		}
	}
}

void UGraphDocument::ReplacePinReferences(const FString& OldPinId, const FString& NewPinId)
{
	if (OldPinId.IsEmpty() || OldPinId == NewPinId)
	{
		return;
	}

	for (FGraphDesignerConnection& Connection : Connections)
	{
		if (Connection.StartPinId == OldPinId)
		{
			Connection.StartPinId = NewPinId;
		}
		if (Connection.EndPinId == OldPinId)
		{
			Connection.EndPinId = NewPinId;
		}
	}

	for (FGraphDesignerRequiredConnection& RequiredConnection : RequiredConnections)
	{
		if (RequiredConnection.StartPinId == OldPinId)
		{
			RequiredConnection.StartPinId = NewPinId;
		}
		if (RequiredConnection.EndPinId == OldPinId)
		{
			RequiredConnection.EndPinId = NewPinId;
		}
	}
}

bool UGraphDocument::PinExists(const FString& PinId) const
{
	return FindPin(PinId) != nullptr;
}
