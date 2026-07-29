#include "GraphCanvasWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Widgets/Layout/SBox.h"

UGraphCanvasWidget::UGraphCanvasWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}
}

void UGraphCanvasWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureDocument();

	if (bLoadSavedGraphOnConstruct && !SavedGraphName.IsEmpty())
	{
		LoadSavedGraphByName(SavedGraphName);
	}
}

void UGraphCanvasWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UGraphCanvasWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	GraphCanvas.Reset();
}

void UGraphCanvasWidget::SetDocument(UGraphDocument* InDocument)
{
	Document = InDocument;
	EnsureDocument();

	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetDocument(Document);
	}
}

void UGraphCanvasWidget::CreateNewGraph()
{
	Document = NewObject<UGraphDocument>(this);
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetDocument(Document);
	}
}

FString UGraphCanvasWidget::AddElement(EGraphDesignerElementType Type, FVector2D Position, FVector2D Size)
{
	EnsureDocument();
	const FString ElementId = Document->AddElement(Type, Position, Size);
	InvalidateCanvas();
	return ElementId;
}

FString UGraphCanvasWidget::AddStyledElement(EGraphDesignerElementType Type, FVector2D Position, FVector2D Size, FLinearColor FillColor, FLinearColor BorderColor)
{
	EnsureDocument();
	const FString ElementId = Document->AddElement(Type, Position, Size);
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->FillColor = FillColor;
		Element->BorderColor = BorderColor;
	}
	InvalidateCanvas();
	return ElementId;
}

bool UGraphCanvasWidget::SetElementText(const FString& ElementId, const FString& Text)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->Text = Text;
		Document->RefreshElementPinIds(ElementId);
		InvalidateCanvas();
		return true;
	}
	return false;
}

bool UGraphCanvasWidget::SetElementImagePath(const FString& ElementId, const FString& ImagePath)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->ImagePath = ImagePath;
		InvalidateCanvas();
		return true;
	}
	return false;
}

bool UGraphCanvasWidget::SetElementTransform(const FString& ElementId, FVector2D Position, FVector2D Size, float Rotation)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->Position = Position;
		Element->Size = Size;
		Element->Rotation = Rotation;
		InvalidateCanvas();
		return true;
	}
	return false;
}

FString UGraphCanvasWidget::AddPinToElement(const FString& ElementId, FVector2D RelativePosition, const FString& PinText)
{
	EnsureDocument();
	const FString PinId = Document->AddPinToElement(ElementId, RelativePosition, PinText);
	InvalidateCanvas();
	return PinId;
}

FString UGraphCanvasWidget::AddConnection(const FString& StartPinId, const FString& EndPinId)
{
	EnsureDocument();
	const FString ConnectionId = Document->AddConnection(StartPinId, EndPinId);
	InvalidateCanvas();
	return ConnectionId;
}

bool UGraphCanvasWidget::DisconnectConnectionByPins(const FString& StartPinId, const FString& EndPinId)
{
	EnsureDocument();
	const bool bRemoved = Document->RemoveConnectionBetweenPins(StartPinId, EndPinId);
	if (bRemoved)
	{
		Document->ClearSelection();
		InvalidateCanvas();
	}
	return bRemoved;
}

bool UGraphCanvasWidget::SetConnectionLineStyle(const FString& ConnectionId, EGraphDesignerConnectionLineStyle LineStyle)
{
	EnsureDocument();
	const bool bUpdated = Document->SetConnectionLineStyle(ConnectionId, LineStyle);
	if (bUpdated)
	{
		InvalidateCanvas();
	}
	return bUpdated;
}

void UGraphCanvasWidget::SetRequiredConnections(const TArray<FGraphDesignerRequiredConnection>& InRequiredConnections)
{
	EnsureDocument();
	Document->SetRequiredConnections(InRequiredConnections);
}

TArray<FGraphDesignerRequiredConnection> UGraphCanvasWidget::GetRequiredConnections() const
{
	return Document != nullptr ? Document->GetRequiredConnections() : TArray<FGraphDesignerRequiredConnection>();
}

int32 UGraphCanvasWidget::AddRequiredConnection(const FString& StartPinId, const FString& EndPinId)
{
	EnsureDocument();
	return Document->AddRequiredConnection(StartPinId, EndPinId);
}

bool UGraphCanvasWidget::UpdateRequiredConnection(int32 Index, const FString& StartPinId, const FString& EndPinId)
{
	EnsureDocument();
	return Document->UpdateRequiredConnection(Index, StartPinId, EndPinId);
}

bool UGraphCanvasWidget::RemoveRequiredConnectionAt(int32 Index)
{
	EnsureDocument();
	return Document->RemoveRequiredConnectionAt(Index);
}

bool UGraphCanvasWidget::IsRequiredConnection(const FString& StartPinId, const FString& EndPinId) const
{
	return Document != nullptr && Document->IsRequiredConnection(StartPinId, EndPinId);
}

bool UGraphCanvasWidget::IsRequiredConnectionUnDirected(const FString& StartPinId, const FString& EndPinId, FString& AnswerStartPinId, FString& AnswerEndPinId) const
{
	return Document != nullptr && Document->IsRequiredConnectionUndirected(StartPinId, EndPinId, AnswerStartPinId, AnswerEndPinId);
}

void UGraphCanvasWidget::SetConnectionRoutingMode(EGraphDesignerConnectionRoutingMode InRoutingMode)
{
	ConnectionRoutingMode = InRoutingMode;
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetConnectionRoutingMode(ConnectionRoutingMode);
	}
}

void UGraphCanvasWidget::ClearSelection()
{
	EnsureDocument();
	Document->ClearSelection();
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->ClearSelection();
	}
	InvalidateCanvas();
}

void UGraphCanvasWidget::SetGraphEditingLocked(bool bLocked)
{
	bGraphEditingLocked = bLocked;
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetGraphEditingLocked(bGraphEditingLocked);
	}
}

void UGraphCanvasWidget::SetZoom(float InZoom)
{
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetZoom(InZoom);
	}
}

float UGraphCanvasWidget::GetZoom() const
{
	return GraphCanvas.IsValid() ? GraphCanvas->GetZoom() : 1.0f;
}

void UGraphCanvasWidget::SetCanvasPan(FVector2D InPan)
{
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetCanvasPan(InPan);
	}
}

FVector2D UGraphCanvasWidget::GetCanvasPan() const
{
	return GraphCanvas.IsValid() ? GraphCanvas->GetCanvasPan() : FVector2D::ZeroVector;
}

bool UGraphCanvasWidget::SaveGraph(const FString& GraphName)
{
	EnsureDocument();
	return Document->SaveToFile(GraphName);
}

bool UGraphCanvasWidget::LoadGraph(const FString& GraphName)
{
	EnsureDocument();
	SavedGraphName = GraphName;
	const bool bLoaded = Document->LoadFromFile(GraphName);
	InvalidateCanvas();
	return bLoaded;
}

bool UGraphCanvasWidget::LoadSavedGraphByName(const FString& GraphName)
{
	return LoadGraph(GraphName);
}

TSharedRef<SWidget> UGraphCanvasWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	EnsureDocument();
	GraphCanvas = SNew(SGraphDesignerCanvas)
		.Document(Document)
		.BackgroundColor(BackgroundColor)
		.GridColor(GridColor)
		.SelectionColor(SelectionColor)
		.bGraphEditingLocked(bGraphEditingLocked)
		.ConnectionRoutingMode(ConnectionRoutingMode)
		.OnConnectionCreated_Lambda([this](const FString& StartPinId, const FString& EndPinId)
		{
			OnConnectionCreated.Broadcast(StartPinId, EndPinId);
			OnConnectionCreatedByName.Broadcast(ResolvePinName(StartPinId), ResolvePinName(EndPinId));
		})
		.OnSelectionChanged_Lambda([this](const TArray<FString>& SelectedElementIds)
		{
			OnSelectionChanged.Broadcast(SelectedElementIds);
		});

	return SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			GraphCanvas.ToSharedRef()
		];
}

void UGraphCanvasWidget::EnsureDocument()
{
	if (Document == nullptr)
	{
		Document = NewObject<UGraphDocument>(this);
	}
}

void UGraphCanvasWidget::InvalidateCanvas()
{
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->Invalidate(EInvalidateWidget::Paint);
	}
}

FString UGraphCanvasWidget::ResolvePinName(const FString& PinId) const
{
	if (Document == nullptr)
	{
		return PinId;
	}

	const FGraphDesignerPin* Pin = Document->FindPin(PinId);
	if (Pin == nullptr || Pin->Id.IsEmpty())
	{
		return PinId;
	}

	return Pin->Id;
}
