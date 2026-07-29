#include "SGraphDesignerCanvas.h"

#include "HAL/FileManager.h"
#include "Engine/Texture2D.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "GraphDesignerDragDropOp.h"
#include "IImageWrapperModule.h"
#include "Misc/Guid.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"

namespace GraphDesignerCanvas
{
	constexpr float MinZoom = 0.1f;
	constexpr float MaxZoom = 4.0f;
	constexpr float PinHitRadius = 10.0f;
	constexpr float PinDropSnapDistance = 18.0f;
	constexpr float ManualRoutePointMinScreenDistance = 16.0f;
	constexpr float ResizeHandleSize = 12.0f;
	constexpr float ResizeHandleHitSize = 18.0f;
	constexpr float ConnectionHitTolerance = 8.0f;
	constexpr float MinElementSize = 20.0f;
	constexpr float BaseGridSize = 64.0f;
	constexpr int32 EllipseSegmentCount = 48;

	FString MakeDuplicateId(const TCHAR* Prefix)
	{
		return FString::Printf(TEXT("%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
	}
}

void SGraphDesignerCanvas::Construct(const FArguments& InArgs)
{
	Document = InArgs._Document;
	BackgroundColor = InArgs._BackgroundColor.Get();
	GridColor = InArgs._GridColor.Get();
	SelectionColor = InArgs._SelectionColor.Get();
	bGraphEditingLocked = InArgs._bGraphEditingLocked;
	ConnectionRoutingMode = InArgs._ConnectionRoutingMode;
	OnConnectionCreated = InArgs._OnConnectionCreated;
	OnSelectionChanged = InArgs._OnSelectionChanged;
	OnPinSelected = InArgs._OnPinSelected;
	OnConnectionSelected = InArgs._OnConnectionSelected;

	ChildSlot
	[
		SNew(SBox)
	];
}

SGraphDesignerCanvas::~SGraphDesignerCanvas()
{
	for (TPair<FString, UTexture2D*>& Pair : ImageTextureCache)
	{
		if (Pair.Value != nullptr)
		{
			Pair.Value->RemoveFromRoot();
		}
	}
	ImageTextureCache.Empty();
	ImageBrushCache.Empty();
	ShapeBrushCache.Empty();
}

void SGraphDesignerCanvas::SetDocument(UGraphDocument* InDocument)
{
	Document = InDocument;
	InteractionMode = EInteractionMode::None;
	ActiveElementId.Reset();
	ResetPendingConnection();
	Invalidate(EInvalidateWidget::Paint);
}

void SGraphDesignerCanvas::SetZoom(float InZoom)
{
	Zoom = FMath::Clamp(InZoom, GraphDesignerCanvas::MinZoom, GraphDesignerCanvas::MaxZoom);
	Invalidate(EInvalidateWidget::Paint);
}

void SGraphDesignerCanvas::SetCanvasPan(FVector2D InPan)
{
	CanvasPan = InPan;
	Invalidate(EInvalidateWidget::Paint);
}

void SGraphDesignerCanvas::SetGraphEditingLocked(bool bLocked)
{
	bGraphEditingLocked = bLocked;
	if (bGraphEditingLocked)
	{
		InteractionMode = EInteractionMode::None;
		ActiveResizeHandle = EGraphDesignerResizeHandle::None;
		ActiveElementId.Reset();
		ResetPendingConnection();
		if (UGraphDocument* GraphDocument = Document.Get())
		{
			GraphDocument->ClearSelection();
			BroadcastSelectionChanged();
		}
		OnPinSelected.ExecuteIfBound(TEXT(""));
		OnConnectionSelected.ExecuteIfBound(TEXT(""));
	}
	Invalidate(EInvalidateWidget::Paint);
}

void SGraphDesignerCanvas::SetConnectionRoutingMode(EGraphDesignerConnectionRoutingMode InRoutingMode)
{
	ConnectionRoutingMode = InRoutingMode;
	ResetPendingConnection();
	Invalidate(EInvalidateWidget::Paint);
}

void SGraphDesignerCanvas::ClearSelection()
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		GraphDocument->ClearSelection();
		BroadcastSelectionChanged();
		OnPinSelected.ExecuteIfBound(TEXT(""));
		OnConnectionSelected.ExecuteIfBound(TEXT(""));
		Invalidate(EInvalidateWidget::Paint);
	}
}

FVector2D SGraphDesignerCanvas::GraphToScreen(const FVector2D& GraphPosition) const
{
	return GraphPosition * Zoom + CanvasPan;
}

FVector2D SGraphDesignerCanvas::ScreenToGraph(const FVector2D& LocalPosition) const
{
	return (LocalPosition - CanvasPan) / Zoom;
}

int32 SGraphDesignerCanvas::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled
) const
{
	DrawBackground(OutDrawElements, AllottedGeometry, LayerId);
	DrawGrid(OutDrawElements, AllottedGeometry, LayerId + 1);

	const int32 ElementCount = Document.IsValid() ? Document->Elements.Num() : 0;
	constexpr int32 ElementLayerStride = 10;
	const int32 ElementBaseLayer = LayerId + 2;
	const int32 ElementLayerSpan = ElementCount * ElementLayerStride;
	const int32 ConnectionsLayer = ElementBaseLayer + ElementLayerSpan;
	const int32 ConnectionPreviewLayer = ConnectionsLayer + 1;
	const int32 PinsBaseLayer = ConnectionPreviewLayer + 1;
	const int32 PinLayerSpan = ElementCount * ElementLayerStride;

	DrawElements(OutDrawElements, AllottedGeometry, ElementBaseLayer);
	DrawConnections(OutDrawElements, AllottedGeometry, ConnectionsLayer);
	DrawConnectionPreview(OutDrawElements, AllottedGeometry, ConnectionPreviewLayer);
	DrawAllPins(OutDrawElements, AllottedGeometry, PinsBaseLayer);

	const int32 SelectionBaseLayer = PinsBaseLayer + PinLayerSpan;
	if (const UGraphDocument* GraphDocument = Document.Get())
	{
		for (int32 Index = 0; Index < GraphDocument->Elements.Num(); ++Index)
		{
			const FGraphDesignerElement& Element = GraphDocument->Elements[Index];
			if (Element.bSelected && !bGraphEditingLocked)
			{
				DrawSelectionAndHandles(
					OutDrawElements,
					AllottedGeometry,
					Element,
					SelectionBaseLayer + Index * ElementLayerStride);
			}
		}
	}

	const int32 MarqueeLayer = SelectionBaseLayer + ElementLayerSpan + 1;
	DrawMarquee(OutDrawElements, AllottedGeometry, MarqueeLayer);
	return MarqueeLayer + 1;
}

FReply SGraphDesignerCanvas::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FVector2D GraphPosition = ScreenToGraph(LocalPosition);
	LastMouseLocalPosition = LocalPosition;
	LastMouseGraphPosition = GraphPosition;
	DragStartGraphPosition = GraphPosition;

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return FReply::Unhandled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (InteractionMode == EInteractionMode::CreatingConnection)
		{
			ResetPendingConnection();
			InteractionMode = EInteractionMode::None;
			Invalidate(EInvalidateWidget::Paint);
			return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
		}

		if (!bGraphEditingLocked)
		{
			if (FGraphDesignerConnection* HitGraphConnection = HitConnection(GraphPosition))
			{
				const FString ConnectionId = HitGraphConnection->Id;
				if (GraphDocument->RemoveConnection(ConnectionId))
				{
					BroadcastSelectionChanged();
					OnConnectionSelected.ExecuteIfBound(TEXT(""));
					Invalidate(EInvalidateWidget::Paint);
				}
				return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
			}
		}

		InteractionMode = EInteractionMode::Panning;
		return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
	{
		InteractionMode = EInteractionMode::Panning;
		return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	FGraphDesignerElement* PinOwnerElement = nullptr;
	if (FGraphDesignerPin* HitGraphPin = HitPin(GraphPosition, &PinOwnerElement))
	{
		GraphDocument->ClearSelection();
		OnConnectionSelected.ExecuteIfBound(TEXT(""));
		HitGraphPin->bSelected = true;
		OnPinSelected.ExecuteIfBound(HitGraphPin->Id);

		if (PendingConnectionStartPinId.IsEmpty())
		{
			PendingConnectionStartPinId = HitGraphPin->Id;
			PendingRoutePoints.Reset();
			InteractionMode = EInteractionMode::CreatingConnection;
		}
		else if (PendingConnectionStartPinId != HitGraphPin->Id)
		{
			const FString StartPinId = PendingConnectionStartPinId;
			const FString EndPinId = HitGraphPin->Id;
			FString ConnectionId;
			if (ConnectionRoutingMode == EGraphDesignerConnectionRoutingMode::ManualRoutePoints)
			{
				TArray<FVector2D> RoutePoints = PendingRoutePoints;
				FVector2D StartGraph;
				FVector2D EndGraph;
				if (GraphDocument->GetPinWorldPosition(StartPinId, StartGraph) && GraphDocument->GetPinWorldPosition(EndPinId, EndGraph))
				{
					const FVector2D LastAnchor = RoutePoints.Num() > 0 ? RoutePoints.Last() : StartGraph;
					const float MinGraphDistance = GraphDesignerCanvas::ManualRoutePointMinScreenDistance / Zoom;
					if (FVector2D::Distance(LastAnchor, EndGraph) >= MinGraphDistance)
					{
						TArray<FVector2D> FinalSegmentPoints;
						BuildOrthogonalConnectionPoints(LastAnchor, EndGraph, FinalSegmentPoints);
						for (int32 PointIndex = 1; PointIndex < FinalSegmentPoints.Num() - 1; ++PointIndex)
						{
							if (RoutePoints.Num() == 0 || !RoutePoints.Last().Equals(FinalSegmentPoints[PointIndex], KINDA_SMALL_NUMBER))
							{
								RoutePoints.Add(FinalSegmentPoints[PointIndex]);
							}
						}
					}
				}
				ConnectionId = GraphDocument->AddConnectionWithRoutePoints(StartPinId, EndPinId, RoutePoints);
			}
			else
			{
				ConnectionId = GraphDocument->AddConnection(StartPinId, EndPinId);
			}
			if (!ConnectionId.IsEmpty())
			{
				OnConnectionCreated.ExecuteIfBound(StartPinId, EndPinId);
			}
			GraphDocument->ClearSelection();
			ResetPendingConnection();
			InteractionMode = EInteractionMode::None;
		}

		Invalidate(EInvalidateWidget::Paint);
		return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (InteractionMode == EInteractionMode::CreatingConnection && ConnectionRoutingMode == EGraphDesignerConnectionRoutingMode::ManualRoutePoints)
	{
		FVector2D StartGraph;
		if (GraphDocument->GetPinWorldPosition(PendingConnectionStartPinId, StartGraph))
		{
			const FVector2D LastAnchor = PendingRoutePoints.Num() > 0 ? PendingRoutePoints.Last() : StartGraph;
			const float MinGraphDistance = GraphDesignerCanvas::ManualRoutePointMinScreenDistance / Zoom;
			if (FVector2D::Distance(LastAnchor, GraphPosition) < MinGraphDistance)
			{
				Invalidate(EInvalidateWidget::Paint);
				return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
			}

			TArray<FVector2D> SegmentPoints;
			BuildOrthogonalConnectionPoints(LastAnchor, GraphPosition, SegmentPoints);
			for (int32 PointIndex = 1; PointIndex < SegmentPoints.Num(); ++PointIndex)
			{
				if (PendingRoutePoints.Num() == 0 || !PendingRoutePoints.Last().Equals(SegmentPoints[PointIndex], KINDA_SMALL_NUMBER))
				{
					PendingRoutePoints.Add(SegmentPoints[PointIndex]);
				}
			}
		}
		Invalidate(EInvalidateWidget::Paint);
		return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (bGraphEditingLocked)
	{
		return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (FGraphDesignerElement* HitGraphElement = HitElement(GraphPosition))
	{
		if (HitGraphElement->bSelected)
		{
			ActiveResizeHandle = HitResizeHandle(*HitGraphElement, GraphPosition);
			if (ActiveResizeHandle != EGraphDesignerResizeHandle::None)
			{
				InteractionMode = EInteractionMode::ResizingElement;
				ActiveElementId = HitGraphElement->Id;
				DragStartElementPosition = HitGraphElement->Position;
				DragStartElementSize = HitGraphElement->Size;
				return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
			}

			InteractionMode = EInteractionMode::DraggingElements;
			ActiveElementId = HitGraphElement->Id;
			DragStartElementPosition = HitGraphElement->Position;
			DragStartElementSize = HitGraphElement->Size;
			return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
		}

		SelectElement(*HitGraphElement, MouseEvent.IsControlDown() || MouseEvent.IsShiftDown());
		InteractionMode = EInteractionMode::DraggingElements;
		ActiveElementId = HitGraphElement->Id;
		DragStartElementPosition = HitGraphElement->Position;
		DragStartElementSize = HitGraphElement->Size;
		return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (FGraphDesignerConnection* HitGraphConnection = HitConnection(GraphPosition))
	{
		SelectConnection(*HitGraphConnection, MouseEvent.IsControlDown() || MouseEvent.IsShiftDown());
		PendingConnectionStartPinId.Reset();
		InteractionMode = EInteractionMode::None;
		return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	if (!MouseEvent.IsControlDown() && !MouseEvent.IsShiftDown())
	{
		GraphDocument->ClearSelection();
		BroadcastSelectionChanged();
		OnPinSelected.ExecuteIfBound(TEXT(""));
		OnConnectionSelected.ExecuteIfBound(TEXT(""));
	}

	MarqueeStartGraphPosition = GraphPosition;
	MarqueeEndGraphPosition = GraphPosition;
	InteractionMode = EInteractionMode::MarqueeSelecting;
	Invalidate(EInvalidateWidget::Paint);
	return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
}

FReply SGraphDesignerCanvas::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FVector2D GraphPosition = ScreenToGraph(LocalPosition);

	if (InteractionMode == EInteractionMode::Panning)
	{
		CanvasPan += LocalPosition - LastMouseLocalPosition;
	}
	else if (InteractionMode == EInteractionMode::DraggingElements && !bGraphEditingLocked)
	{
		MoveSelectedElements(GraphPosition - LastMouseGraphPosition);
	}
	else if (InteractionMode == EInteractionMode::ResizingElement && !bGraphEditingLocked)
	{
		if (UGraphDocument* GraphDocument = Document.Get())
		{
			if (FGraphDesignerElement* Element = GraphDocument->FindElement(ActiveElementId))
			{
				ResizeElement(*Element, GraphPosition);
			}
		}
	}
	else if (InteractionMode == EInteractionMode::CreatingConnection)
	{
		LastMouseGraphPosition = GraphPosition;
	}
	else if (InteractionMode == EInteractionMode::MarqueeSelecting && !bGraphEditingLocked)
	{
		MarqueeEndGraphPosition = GraphPosition;
		SelectElementsInMarquee();
	}

	LastMouseLocalPosition = LocalPosition;
	LastMouseGraphPosition = GraphPosition;
	Invalidate(EInvalidateWidget::Paint);
	return InteractionMode == EInteractionMode::None ? FReply::Unhandled() : FReply::Handled();
}

FReply SGraphDesignerCanvas::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (HasMouseCapture())
	{
		if (InteractionMode == EInteractionMode::MarqueeSelecting)
		{
			SelectElementsInMarquee();
		}

		if (InteractionMode != EInteractionMode::CreatingConnection)
		{
			InteractionMode = EInteractionMode::None;
		}
		ActiveResizeHandle = EGraphDesignerResizeHandle::None;
		ActiveElementId.Reset();
		Invalidate(EInvalidateWidget::Paint);
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FReply SGraphDesignerCanvas::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FVector2D GraphPositionBeforeZoom = ScreenToGraph(LocalPosition);
	const float ZoomMultiplier = FMath::Pow(1.12f, MouseEvent.GetWheelDelta());
	Zoom = FMath::Clamp(Zoom * ZoomMultiplier, GraphDesignerCanvas::MinZoom, GraphDesignerCanvas::MaxZoom);
	CanvasPan = LocalPosition - GraphPositionBeforeZoom * Zoom;
	Invalidate(EInvalidateWidget::Paint);
	return FReply::Handled();
}

FReply SGraphDesignerCanvas::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.IsControlDown() && InKeyEvent.GetKey() == EKeys::C)
	{
		CopySelectedElementsToClipboard();
		return FReply::Handled();
	}

	if (InKeyEvent.IsControlDown() && InKeyEvent.GetKey() == EKeys::V)
	{
		PasteClipboardElements();
		return FReply::Handled();
	}

	if (InteractionMode == EInteractionMode::CreatingConnection)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			ResetPendingConnection();
			InteractionMode = EInteractionMode::None;
			Invalidate(EInvalidateWidget::Paint);
			return FReply::Handled();
		}

		if (InKeyEvent.GetKey() == EKeys::BackSpace)
		{
			if (PendingRoutePoints.Num() > 0)
			{
				PendingRoutePoints.Pop();
			}
			else
			{
				ResetPendingConnection();
				InteractionMode = EInteractionMode::None;
			}
			Invalidate(EInvalidateWidget::Paint);
			return FReply::Handled();
		}
	}

	if (InKeyEvent.GetKey() == EKeys::Delete || InKeyEvent.GetKey() == EKeys::BackSpace)
	{
		if (bGraphEditingLocked)
		{
			return FReply::Handled();
		}
		DeleteSelectedElements();
		DeleteSelectedConnections();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SGraphDesignerCanvas::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (bGraphEditingLocked)
	{
		return FReply::Unhandled();
	}

	return DragDropEvent.GetOperationAs<FGraphDesignerTemplateDragDropOp>().IsValid()
		? FReply::Handled()
		: FReply::Unhandled();
}

FReply SGraphDesignerCanvas::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (bGraphEditingLocked)
	{
		return FReply::Unhandled();
	}

	const TSharedPtr<FGraphDesignerTemplateDragDropOp> TemplateOperation = DragDropEvent.GetOperationAs<FGraphDesignerTemplateDragDropOp>();
	if (!TemplateOperation.IsValid() || Document.Get() == nullptr)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
	const FVector2D DropGraphPosition = ScreenToGraph(LocalPosition);
	if (TemplateOperation->NodeTemplate.Type == EGraphDesignerElementType::Pin)
	{
		CreatePinFromTemplate(TemplateOperation->NodeTemplate, DropGraphPosition);
	}
	else
	{
		CreateElementFromTemplate(TemplateOperation->NodeTemplate, DropGraphPosition - TemplateOperation->NodeTemplate.DefaultSize * 0.5f);
	}
	return FReply::Handled();
}

void SGraphDesignerCanvas::DrawBackground(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		BackgroundColor
	);
}

void SGraphDesignerCanvas::DrawGrid(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const float GridStep = GraphDesignerCanvas::BaseGridSize * Zoom;
	if (GridStep < 4.0f)
	{
		return;
	}

	const float StartX = FMath::Fmod(CanvasPan.X, GridStep);
	const float StartY = FMath::Fmod(CanvasPan.Y, GridStep);

	TArray<FVector2D> LinePoints;
	for (float X = StartX; X < LocalSize.X; X += GridStep)
	{
		LinePoints.Reset();
		LinePoints.Add(FVector2D(X, 0.0f));
		LinePoints.Add(FVector2D(X, LocalSize.Y));
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, GridColor, true, 1.0f);
	}

	for (float Y = StartY; Y < LocalSize.Y; Y += GridStep)
	{
		LinePoints.Reset();
		LinePoints.Add(FVector2D(0.0f, Y));
		LinePoints.Add(FVector2D(LocalSize.X, Y));
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, GridColor, true, 1.0f);
	}
}

void SGraphDesignerCanvas::DrawConnections(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	const UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	for (const FGraphDesignerConnection& Connection : GraphDocument->Connections)
	{
		FVector2D StartGraph;
		FVector2D EndGraph;
		if (!GraphDocument->GetPinWorldPosition(Connection.StartPinId, StartGraph) || !GraphDocument->GetPinWorldPosition(Connection.EndPinId, EndGraph))
		{
			continue;
		}

		TArray<FVector2D> ConnectionGraphPoints;
		BuildConnectionGraphPoints(Connection, StartGraph, EndGraph, ConnectionGraphPoints);

		TArray<FVector2D> ConnectionScreenPoints;
		ConnectionScreenPoints.Reserve(ConnectionGraphPoints.Num());
		for (const FVector2D& Point : ConnectionGraphPoints)
		{
			ConnectionScreenPoints.Add(GraphToScreen(Point));
		}

		if (Connection.LineStyle == EGraphDesignerConnectionLineStyle::Dashed)
		{
			TArray<FVector2f> DashPoints;
			for (const FVector2D& Point : ConnectionScreenPoints)
			{
				FVector2f TempPoint = FVector2f(Point);
				DashPoints.Add(TempPoint);
			}
			//DrawDashedPolyline(OutDrawElements,AllottedGeometry,LayerId,ConnectionScreenPoints,Connection.bSelected ? SelectionColor : Connection.Color,FMath::Max(Connection.Thickness * Zoom, 1.0f));
			FSlateDrawElement::MakeDashedLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), MoveTemp(DashPoints), ESlateDrawEffect::None, Connection.bSelected ? SelectionColor : Connection.Color,FMath::Max(Connection.Thickness * Zoom, 1.0f), 10.0f, 0.f);
		}
		else
		{
			FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			ConnectionScreenPoints,
			ESlateDrawEffect::None,
			Connection.bSelected ? SelectionColor : Connection.Color,
			true,
			FMath::Max(Connection.Thickness * Zoom, 1.0f)
			);
		}

		
	}
}

void SGraphDesignerCanvas::DrawElements(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	const UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < GraphDocument->Elements.Num(); ++Index)
	{
		const FGraphDesignerElement& Element = GraphDocument->Elements[Index];
		const int32 ElementLayer = LayerId + Index * 10;
		DrawElement(OutDrawElements, AllottedGeometry, Element, ElementLayer);
	}
}

void SGraphDesignerCanvas::DrawElement(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, int32 LayerId) const
{
	const FVector2D ScreenPosition = GraphToScreen(Element.Position);
	const FVector2D ScreenSize = Element.Size * Zoom;
	const FVector2D Center = ScreenPosition + ScreenSize * 0.5f;
	const float Thickness = FMath::Max(1.0f, Zoom);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	if (Element.Type == EGraphDesignerElementType::Line || Element.Type == EGraphDesignerElementType::Arrow)
	{
		const FVector2D Start = ScreenPosition + FVector2D(0.0f, ScreenSize.Y * 0.5f);
		const FVector2D End = RotatePointAround(ScreenPosition + FVector2D(ScreenSize.X, ScreenSize.Y * 0.5f), Start, Element.Rotation);
		TArray<FVector2D> Points;
		Points.Add(Start);
		Points.Add(End);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
		if (Element.Type == EGraphDesignerElementType::Arrow)
		{
			DrawArrowHead(OutDrawElements, AllottedGeometry, LayerId + 1, Start, End, Element.BorderColor, Thickness * 2.0f);
		}
		return;
	}

	FLinearColor Fill = Element.FillColor;
	const FSlateBrush* ElementBrush = WhiteBrush;
	if (Element.Type == EGraphDesignerElementType::Image && !Element.ImagePath.IsEmpty())
	{
		ElementBrush = GetImageBrush(Element);
		Fill = ElementBrush == WhiteBrush ? Fill : FLinearColor::White;
	}

	if (Element.Type == EGraphDesignerElementType::Text)
	{
		DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId);
		return;
	}

	if (Element.Type == EGraphDesignerElementType::Image)
	{
		TArray<FSlateVertex> Vertices;
		Vertices.AddZeroed(4);

		TArray<FVector2D> Points;
		Points.Add(ScreenPosition);
		Points.Add(ScreenPosition + FVector2D(ScreenSize.X, 0.0f));
		Points.Add(ScreenPosition + ScreenSize);
		Points.Add(ScreenPosition + FVector2D(0.0f, ScreenSize.Y));

		if (!FMath::IsNearlyZero(Element.Rotation))
		{
			for (FVector2D& Point : Points)
			{
				Point = RotatePointAround(Point, Center, Element.Rotation);
			}
		}

		const FVector2D UvCorners[4] =
		{
			FVector2D(0.0f, 0.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(1.0f, 1.0f),
			FVector2D(0.0f, 1.0f)
		};

		for (int32 Index = 0; Index < 4; ++Index)
		{
			Vertices[Index].Position[0] = AllottedGeometry.LocalToAbsolute(Points[Index]).X;
			Vertices[Index].Position[1] = AllottedGeometry.LocalToAbsolute(Points[Index]).Y;
			Vertices[Index].TexCoords[0] = UvCorners[Index].X;
			Vertices[Index].TexCoords[1] = UvCorners[Index].Y;
			Vertices[Index].TexCoords[2] = Vertices[Index].TexCoords[3] = 1.0f;
			Vertices[Index].Color = Fill.ToFColor(true);
		}

		TArray<SlateIndex> Indexes = { 0, 1, 2, 0, 2, 3 };
		FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, ElementBrush->GetRenderingResource(), Vertices, Indexes, nullptr, 0, 0);

		TArray<FVector2D> BorderPoints = Points;
		BorderPoints.Add(Points[0]);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), BorderPoints, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
		DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);
		return;
	}

	if (Element.Type == EGraphDesignerElementType::Circle || Element.Type == EGraphDesignerElementType::Ellipse)
	{
		const FVector2D Radius = Element.Type == EGraphDesignerElementType::Circle
			? FVector2D(FMath::Min(ScreenSize.X, ScreenSize.Y) * 0.5f)
			: ScreenSize * 0.5f;
		DrawFilledEllipse(OutDrawElements, AllottedGeometry, LayerId, Center, Radius, Fill);
		DrawEllipseOutline(OutDrawElements, AllottedGeometry, LayerId + 1, Center, Radius, Element.BorderColor, Thickness * 2.0f);
		DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);
		return;
	}

	if (Element.Type == EGraphDesignerElementType::RoundedRectangle)
	{
		const float Radius = FMath::Min(ScreenSize.X, ScreenSize.Y) * 0.18f;
		TArray<FVector2D> RoundedPoints;
		BuildRoundedRectanglePoints(ScreenPosition, ScreenSize, Radius, RoundedPoints);
		DrawFilledPolygon(OutDrawElements, AllottedGeometry, LayerId, RoundedPoints, Fill);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), RoundedPoints, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
		DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);
		return;
	}

	TArray<FVector2D> PolygonPoints;
	if (BuildElementPolygonPoints(Element.Type, ScreenPosition, ScreenSize, PolygonPoints))
	{
		DrawFilledPolygon(OutDrawElements, AllottedGeometry, LayerId, PolygonPoints, Fill);
		TArray<FVector2D> OutlinePoints = PolygonPoints;
		OutlinePoints.Add(PolygonPoints[0]);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), OutlinePoints, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
		DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);
		return;
	}

	if (Element.Type == EGraphDesignerElementType::Rectangle || Element.Type == EGraphDesignerElementType::Pin)
	{
		if (!FMath::IsNearlyZero(Element.Rotation))
		{
			TArray<FVector2D> RotatedPoints;
			RotatedPoints.Add(RotatePointAround(ScreenPosition, Center, Element.Rotation));
			RotatedPoints.Add(RotatePointAround(ScreenPosition + FVector2D(ScreenSize.X, 0.0f), Center, Element.Rotation));
			RotatedPoints.Add(RotatePointAround(ScreenPosition + ScreenSize, Center, Element.Rotation));
			RotatedPoints.Add(RotatePointAround(ScreenPosition + FVector2D(0.0f, ScreenSize.Y), Center, Element.Rotation));
			DrawFilledPolygon(OutDrawElements, AllottedGeometry, LayerId, RotatedPoints, Fill);
			TArray<FVector2D> OutlinePoints = RotatedPoints;
			OutlinePoints.Add(RotatedPoints[0]);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), OutlinePoints, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
			DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);
			return;
		}

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(ScreenPosition, ScreenSize),
			WhiteBrush,
			ESlateDrawEffect::None,
			Fill
		);

		TArray<FVector2D> BorderPoints;
		BorderPoints.Add(ScreenPosition);
		BorderPoints.Add(ScreenPosition + FVector2D(ScreenSize.X, 0.0f));
		BorderPoints.Add(ScreenPosition + ScreenSize);
		BorderPoints.Add(ScreenPosition + FVector2D(0.0f, ScreenSize.Y));
		BorderPoints.Add(ScreenPosition);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), BorderPoints, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
		DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);
		return;
	}

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(ScreenPosition, ScreenSize),
		ElementBrush,
		ESlateDrawEffect::None,
		Fill
	);

	TArray<FVector2D> BorderPoints;
	const FVector2D FirstBorderPoint = RotatePointAround(ScreenPosition, Center, Element.Rotation);
	BorderPoints.Add(FirstBorderPoint);
	BorderPoints.Add(RotatePointAround(ScreenPosition + FVector2D(ScreenSize.X, 0.0f), Center, Element.Rotation));
	BorderPoints.Add(RotatePointAround(ScreenPosition + ScreenSize, Center, Element.Rotation));
	BorderPoints.Add(RotatePointAround(ScreenPosition + FVector2D(0.0f, ScreenSize.Y), Center, Element.Rotation));
	BorderPoints.Add(FirstBorderPoint);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), BorderPoints, ESlateDrawEffect::None, Element.BorderColor, true, Thickness * 2.0f);
	DrawElementText(OutDrawElements, AllottedGeometry, Element, ScreenPosition, ScreenSize, LayerId + 5);

	if (Element.Type == EGraphDesignerElementType::Image && !Element.ImagePath.IsEmpty())
	{
		const FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", FMath::RoundToInt(10.0f * Zoom));
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(ScreenPosition + FVector2D(4.0f, 4.0f), ScreenSize),
			FText::FromString(Element.ImagePath),
			FontInfo,
			ESlateDrawEffect::None,
			Element.BorderColor
		);
	}
}

void SGraphDesignerCanvas::DrawAllPins(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry,
	int32 LayerId) const
{
	const UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < GraphDocument->Elements.Num(); ++Index)
	{
		const FGraphDesignerElement& Element = GraphDocument->Elements[Index];
		const int32 ElementLayer = LayerId + Index * 10;
		DrawPins(OutDrawElements, AllottedGeometry, Element, ElementLayer);
	}
}

void SGraphDesignerCanvas::DrawPins(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, int32 LayerId) const
{
	for (const FGraphDesignerPin& Pin : Element.Pins)
	{
		const FVector2D PinCenter = GraphToScreen(Element.Position + Pin.RelativePosition);
		const FVector2D PinSize = Pin.Size * Zoom;
		const FVector2D PinTopLeft = PinCenter - PinSize * 0.5f;
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(PinTopLeft, PinSize),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			Pin.bSelected ? SelectionColor : Pin.FillColor
		);

		TArray<FVector2D> Outline;
		Outline.Add(PinTopLeft);
		Outline.Add(PinTopLeft + FVector2D(PinSize.X, 0.0f));
		Outline.Add(PinTopLeft + PinSize);
		Outline.Add(PinTopLeft + FVector2D(0.0f, PinSize.Y));
		Outline.Add(PinTopLeft);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), Outline, ESlateDrawEffect::None, Pin.BorderColor, true, 1.0f);
		DrawPinText(OutDrawElements, AllottedGeometry, Pin, PinCenter, PinSize, LayerId + 2);
	}
}

void SGraphDesignerCanvas::DrawConnectionPreview(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	const UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr || PendingConnectionStartPinId.IsEmpty() || InteractionMode != EInteractionMode::CreatingConnection)
	{
		return;
	}

	FVector2D StartGraph;
	if (!GraphDocument->GetPinWorldPosition(PendingConnectionStartPinId, StartGraph))
	{
		return;
	}

	TArray<FVector2D> GraphPoints;
	if (ConnectionRoutingMode == EGraphDesignerConnectionRoutingMode::ManualRoutePoints)
	{
		BuildManualPreviewGraphPoints(StartGraph, LastMouseGraphPosition, GraphPoints);
	}
	else
	{
		BuildOrthogonalConnectionPoints(StartGraph, LastMouseGraphPosition, GraphPoints);
	}

	TArray<FVector2D> ScreenPoints;
	ScreenPoints.Reserve(GraphPoints.Num());
	for (const FVector2D& Point : GraphPoints)
	{
		ScreenPoints.Add(GraphToScreen(Point));
	}

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), ScreenPoints, ESlateDrawEffect::None, SelectionColor, true, FMath::Max(1.0f, 2.0f * Zoom));
}

void SGraphDesignerCanvas::DrawSelectionAndHandles(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, int32 LayerId) const
{
	const FVector2D ScreenPosition = GraphToScreen(Element.Position);
	const FVector2D ScreenSize = Element.Size * Zoom;
	TArray<FVector2D> Border;
	Border.Add(ScreenPosition);
	Border.Add(ScreenPosition + FVector2D(ScreenSize.X, 0.0f));
	Border.Add(ScreenPosition + ScreenSize);
	Border.Add(ScreenPosition + FVector2D(0.0f, ScreenSize.Y));
	Border.Add(ScreenPosition);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Border, ESlateDrawEffect::None, SelectionColor, true, 1.5f);

	const FVector2D HandleSize(GraphDesignerCanvas::ResizeHandleSize);
	const TArray<FVector2D> Handles =
	{
		ScreenPosition,
		ScreenPosition + FVector2D(ScreenSize.X * 0.5f, 0.0f),
		ScreenPosition + FVector2D(ScreenSize.X, 0.0f),
		ScreenPosition + FVector2D(ScreenSize.X, ScreenSize.Y * 0.5f),
		ScreenPosition + ScreenSize,
		ScreenPosition + FVector2D(ScreenSize.X * 0.5f, ScreenSize.Y),
		ScreenPosition + FVector2D(0.0f, ScreenSize.Y),
		ScreenPosition + FVector2D(0.0f, ScreenSize.Y * 0.5f)
	};

	for (const FVector2D& HandleCenter : Handles)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(HandleCenter - HandleSize * 0.5f, HandleSize),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			SelectionColor
		);
	}
}

void SGraphDesignerCanvas::DrawMarquee(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	if (InteractionMode != EInteractionMode::MarqueeSelecting)
	{
		return;
	}

	const FVector2D A = GraphToScreen(MarqueeStartGraphPosition);
	const FVector2D B = GraphToScreen(MarqueeEndGraphPosition);
	const FVector2D TopLeft(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
	const FVector2D BottomRight(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
	const FVector2D Size = BottomRight - TopLeft;

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(TopLeft, Size),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		FLinearColor(SelectionColor.R, SelectionColor.G, SelectionColor.B, 0.12f)
	);
}

void SGraphDesignerCanvas::BuildOrthogonalConnectionPoints(const FVector2D& Start, const FVector2D& End, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();

	auto AddPoint = [&OutPoints](const FVector2D& Point)
	{
		if (OutPoints.Num() == 0 || !OutPoints.Last().Equals(Point, KINDA_SMALL_NUMBER))
		{
			OutPoints.Add(Point);
		}
	};

	const float MidX = (Start.X + End.X) * 0.5f;
	AddPoint(Start);
	AddPoint(FVector2D(MidX, Start.Y));
	AddPoint(FVector2D(MidX, End.Y));
	AddPoint(End);
}

void SGraphDesignerCanvas::BuildConnectionGraphPoints(const FGraphDesignerConnection& Connection, const FVector2D& StartGraph, const FVector2D& EndGraph, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();
	if (Connection.RoutePoints.Num() == 0)
	{
		BuildOrthogonalConnectionPoints(StartGraph, EndGraph, OutPoints);
		return;
	}

	OutPoints.Reserve(Connection.RoutePoints.Num() + 2);
	OutPoints.Add(StartGraph);
	for (const FVector2D& RoutePoint : Connection.RoutePoints)
	{
		if (!OutPoints.Last().Equals(RoutePoint, KINDA_SMALL_NUMBER))
		{
			OutPoints.Add(RoutePoint);
		}
	}
	if (!OutPoints.Last().Equals(EndGraph, KINDA_SMALL_NUMBER))
	{
		OutPoints.Add(EndGraph);
	}
}

void SGraphDesignerCanvas::BuildManualPreviewGraphPoints(const FVector2D& StartGraph, const FVector2D& EndGraph, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();
	TArray<FVector2D> FixedPoints;
	FixedPoints.Reserve(PendingRoutePoints.Num() + 1);
	FixedPoints.Add(StartGraph);
	for (const FVector2D& RoutePoint : PendingRoutePoints)
	{
		if (!FixedPoints.Last().Equals(RoutePoint, KINDA_SMALL_NUMBER))
		{
			FixedPoints.Add(RoutePoint);
		}
	}

	for (const FVector2D& Point : FixedPoints)
	{
		if (OutPoints.Num() == 0 || !OutPoints.Last().Equals(Point, KINDA_SMALL_NUMBER))
		{
			OutPoints.Add(Point);
		}
	}

	const float MinGraphDistance = GraphDesignerCanvas::ManualRoutePointMinScreenDistance / Zoom;
	if (FVector2D::Distance(FixedPoints.Last(), EndGraph) < MinGraphDistance)
	{
		return;
	}

	TArray<FVector2D> PreviewSegmentPoints;
	BuildOrthogonalConnectionPoints(FixedPoints.Last(), EndGraph, PreviewSegmentPoints);
	for (int32 PointIndex = 1; PointIndex < PreviewSegmentPoints.Num(); ++PointIndex)
	{
		if (OutPoints.Num() == 0 || !OutPoints.Last().Equals(PreviewSegmentPoints[PointIndex], KINDA_SMALL_NUMBER))
		{
			OutPoints.Add(PreviewSegmentPoints[PointIndex]);
		}
	}
}

void SGraphDesignerCanvas::ResetPendingConnection()
{
	PendingConnectionStartPinId.Reset();
	PendingRoutePoints.Reset();
	OnPinSelected.ExecuteIfBound(TEXT(""));
}

void SGraphDesignerCanvas::DrawFilledPolygon(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const TArray<FVector2D>& Points, const FLinearColor& FillColor) const
{
	if (Points.Num() < 3)
	{
		return;
	}

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	if (WhiteBrush == nullptr)
	{
		return;
	}

	FVector2D Center = FVector2D::ZeroVector;
	for (const FVector2D& Point : Points)
	{
		Center += Point;
	}
	Center /= static_cast<float>(Points.Num());

	TArray<FSlateVertex> Vertices;
	Vertices.Reserve(Points.Num() + 1);

	Vertices.AddZeroed();
	FSlateVertex& CenterVertex = Vertices.Last();
	const FVector2D AbsoluteCenter = AllottedGeometry.LocalToAbsolute(Center);
	CenterVertex.Position[0] = AbsoluteCenter.X;
	CenterVertex.Position[1] = AbsoluteCenter.Y;
	CenterVertex.TexCoords[0] = 0.5f;
	CenterVertex.TexCoords[1] = 0.5f;
	CenterVertex.TexCoords[2] = CenterVertex.TexCoords[3] = 1.0f;
	CenterVertex.Color = FillColor.ToFColor(true);

	for (const FVector2D& Point : Points)
	{
		Vertices.AddZeroed();
		FSlateVertex& Vertex = Vertices.Last();
		const FVector2D AbsolutePosition = AllottedGeometry.LocalToAbsolute(Point);
		Vertex.Position[0] = AbsolutePosition.X;
		Vertex.Position[1] = AbsolutePosition.Y;
		Vertex.TexCoords[0] = 0.5f;
		Vertex.TexCoords[1] = 0.5f;
		Vertex.TexCoords[2] = Vertex.TexCoords[3] = 1.0f;
		Vertex.Color = FillColor.ToFColor(true);
	}

	TArray<SlateIndex> Indexes;
	Indexes.Reserve(Points.Num() * 3);
	for (int32 Index = 1; Index <= Points.Num(); ++Index)
	{
		Indexes.Add(0);
		Indexes.Add(Index);
		Indexes.Add(Index == Points.Num() ? 1 : Index + 1);
	}

	FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, WhiteBrush->GetRenderingResource(), Vertices, Indexes, nullptr, 0, 0);
}

void SGraphDesignerCanvas::DrawFilledEllipse(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Center, const FVector2D& Radius, const FLinearColor& FillColor) const
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	if (WhiteBrush == nullptr)
	{
		return;
	}

	TArray<FSlateVertex> Vertices;
	Vertices.Reserve(GraphDesignerCanvas::EllipseSegmentCount + 1);
	const FVector2D AbsoluteCenter = AllottedGeometry.LocalToAbsolute(Center);

	Vertices.AddZeroed();
	FSlateVertex& CenterVertex = Vertices.Last();
	CenterVertex.Position[0] = AbsoluteCenter.X;
	CenterVertex.Position[1] = AbsoluteCenter.Y;
	CenterVertex.TexCoords[0] = 0.5f;
	CenterVertex.TexCoords[1] = 0.5f;
	CenterVertex.TexCoords[2] = CenterVertex.TexCoords[3] = 1.0f;
	CenterVertex.Color = FillColor.ToFColor(true);

	for (int32 Index = 0; Index < GraphDesignerCanvas::EllipseSegmentCount; ++Index)
	{
		const float Angle = static_cast<float>(Index) / static_cast<float>(GraphDesignerCanvas::EllipseSegmentCount) * 2.0f * PI;
		const FVector2D AbsolutePosition = AllottedGeometry.LocalToAbsolute(FVector2D(Center.X + FMath::Cos(Angle) * Radius.X, Center.Y + FMath::Sin(Angle) * Radius.Y));
		Vertices.AddZeroed();
		FSlateVertex& Vertex = Vertices.Last();
		Vertex.Position[0] = AbsolutePosition.X;
		Vertex.Position[1] = AbsolutePosition.Y;
		Vertex.TexCoords[0] = 0.5f;
		Vertex.TexCoords[1] = 0.5f;
		Vertex.TexCoords[2] = Vertex.TexCoords[3] = 1.0f;
		Vertex.Color = FillColor.ToFColor(true);
	}

	TArray<SlateIndex> Indexes;
	Indexes.Reserve(GraphDesignerCanvas::EllipseSegmentCount * 3);
	for (int32 Index = 1; Index <= GraphDesignerCanvas::EllipseSegmentCount; ++Index)
	{
		Indexes.Add(0);
		Indexes.Add(Index);
		Indexes.Add(Index == GraphDesignerCanvas::EllipseSegmentCount ? 1 : Index + 1);
	}

	FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, WhiteBrush->GetRenderingResource(), Vertices, Indexes, nullptr, 0, 0);
}

void SGraphDesignerCanvas::DrawEllipseOutline(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Center, const FVector2D& Radius, const FLinearColor& Color, float Thickness) const
{
	TArray<FVector2D> OutlinePoints;
	OutlinePoints.Reserve(GraphDesignerCanvas::EllipseSegmentCount + 1);
	for (int32 Index = 0; Index <= GraphDesignerCanvas::EllipseSegmentCount; ++Index)
	{
		const float Angle = static_cast<float>(Index) / static_cast<float>(GraphDesignerCanvas::EllipseSegmentCount) * 2.0f * PI;
		OutlinePoints.Add(FVector2D(Center.X + FMath::Cos(Angle) * Radius.X, Center.Y + FMath::Sin(Angle) * Radius.Y));
	}

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), OutlinePoints, ESlateDrawEffect::None, Color, true, Thickness);
}

bool SGraphDesignerCanvas::BuildElementPolygonPoints(EGraphDesignerElementType Type, const FVector2D& Position, const FVector2D& Size, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();
	const FVector2D Center = Position + Size * 0.5f;

	switch (Type)
	{
	case EGraphDesignerElementType::Triangle:
		OutPoints.Add(Position + FVector2D(Size.X * 0.5f, 0.0f));
		OutPoints.Add(Position + Size);
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y));
		return true;
	case EGraphDesignerElementType::RightTriangle:
		OutPoints.Add(Position);
		OutPoints.Add(Position + Size);
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y));
		return true;
	case EGraphDesignerElementType::Diamond:
		OutPoints.Add(Position + FVector2D(Size.X * 0.5f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X, Size.Y * 0.5f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.5f, Size.Y));
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y * 0.5f));
		return true;
	case EGraphDesignerElementType::Parallelogram:
		OutPoints.Add(Position + FVector2D(Size.X * 0.25f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.75f, Size.Y));
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y));
		return true;
	case EGraphDesignerElementType::Trapezoid:
		OutPoints.Add(Position + FVector2D(Size.X * 0.25f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.75f, 0.0f));
		OutPoints.Add(Position + Size);
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y));
		return true;
	case EGraphDesignerElementType::Pentagon:
		OutPoints.Add(Position + FVector2D(Size.X * 0.5f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X, Size.Y * 0.38f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.82f, Size.Y));
		OutPoints.Add(Position + FVector2D(Size.X * 0.18f, Size.Y));
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y * 0.38f));
		return true;
	case EGraphDesignerElementType::Hexagon:
		OutPoints.Add(Position + FVector2D(Size.X * 0.25f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.75f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X, Size.Y * 0.5f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.75f, Size.Y));
		OutPoints.Add(Position + FVector2D(Size.X * 0.25f, Size.Y));
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y * 0.5f));
		return true;
	case EGraphDesignerElementType::Octagon:
		OutPoints.Add(Position + FVector2D(Size.X * 0.30f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.70f, 0.0f));
		OutPoints.Add(Position + FVector2D(Size.X, Size.Y * 0.30f));
		OutPoints.Add(Position + FVector2D(Size.X, Size.Y * 0.70f));
		OutPoints.Add(Position + FVector2D(Size.X * 0.70f, Size.Y));
		OutPoints.Add(Position + FVector2D(Size.X * 0.30f, Size.Y));
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y * 0.70f));
		OutPoints.Add(Position + FVector2D(0.0f, Size.Y * 0.30f));
		return true;
	case EGraphDesignerElementType::Star:
		for (int32 Index = 0; Index < 10; ++Index)
		{
			const float RadiusScale = (Index % 2 == 0) ? 0.5f : 0.22f;
			const float Angle = -HALF_PI + static_cast<float>(Index) / 10.0f * 2.0f * PI;
			OutPoints.Add(Center + FVector2D(FMath::Cos(Angle) * Size.X * RadiusScale, FMath::Sin(Angle) * Size.Y * RadiusScale));
		}
		return true;
	default:
		return false;
	}
}

void SGraphDesignerCanvas::BuildRoundedRectanglePoints(const FVector2D& Position, const FVector2D& Size, float Radius, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();
	const float ClampedRadius = FMath::Clamp(Radius, 0.0f, FMath::Min(Size.X, Size.Y) * 0.5f);
	const int32 CornerSegments = 8;

	const FVector2D TopLeft = Position + FVector2D(ClampedRadius, ClampedRadius);
	const FVector2D TopRight = Position + FVector2D(Size.X - ClampedRadius, ClampedRadius);
	const FVector2D BottomRight = Position + FVector2D(Size.X - ClampedRadius, Size.Y - ClampedRadius);
	const FVector2D BottomLeft = Position + FVector2D(ClampedRadius, Size.Y - ClampedRadius);

	auto AddCorner = [&OutPoints, ClampedRadius, CornerSegments](const FVector2D& Center, float StartAngle, float EndAngle)
	{
		for (int32 Index = 0; Index <= CornerSegments; ++Index)
		{
			const float Alpha = static_cast<float>(Index) / static_cast<float>(CornerSegments);
			const float Angle = FMath::Lerp(StartAngle, EndAngle, Alpha);
			OutPoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * ClampedRadius);
		}
	};

	AddCorner(TopRight, -HALF_PI, 0.0f);
	AddCorner(BottomRight, 0.0f, HALF_PI);
	AddCorner(BottomLeft, HALF_PI, PI);
	AddCorner(TopLeft, PI, PI + HALF_PI);

	if (OutPoints.Num() > 0)
	{
		const FVector2D FirstPoint = OutPoints[0];
		OutPoints.Add(FirstPoint);
	}
}

void SGraphDesignerCanvas::DrawElementText(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, const FVector2D& ScreenPosition, const FVector2D& ScreenSize, int32 LayerId) const
{
	if (Element.Text.IsEmpty())
	{
		return;
	}

	const int32 ScaledFontSize = FMath::Max(1, FMath::RoundToInt(Element.FontSize * Zoom));
	const FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", ScaledFontSize);
	const FString DisplayText = NormalizeDisplayText(Element.Text);
	const FVector2D TextSize = MeasureMultilineText(DisplayText, FontInfo);
	const FVector2D TextPosition = ScreenPosition + (ScreenSize - TextSize) * 0.5f;

	DrawMultilineText(OutDrawElements, AllottedGeometry, LayerId, DisplayText, FontInfo, TextPosition, Element.TextColor);
}

void SGraphDesignerCanvas::DrawPinText(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerPin& Pin, const FVector2D& PinCenter, const FVector2D& PinSize, int32 LayerId) const
{
	if (Pin.Text.IsEmpty())
	{
		return;
	}

	constexpr float TextPadding = 4.0f;
	const int32 ScaledFontSize = FMath::Max(1, FMath::RoundToInt(Pin.FontSize * Zoom));
	const FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", ScaledFontSize);
	const FString DisplayText = NormalizeDisplayText(Pin.Text);
	const FVector2D TextSize = MeasureMultilineText(DisplayText, FontInfo);

	FVector2D TextPosition = PinCenter;
	switch (Pin.TextPosition)
	{
	case EGraphDesignerPinTextPosition::Left:
		TextPosition = PinCenter + FVector2D(-PinSize.X * 0.5f - TextPadding - TextSize.X, -TextSize.Y * 0.5f);
		break;
	case EGraphDesignerPinTextPosition::Right:
		TextPosition = PinCenter + FVector2D(PinSize.X * 0.5f + TextPadding, -TextSize.Y * 0.5f);
		break;
	case EGraphDesignerPinTextPosition::Top:
		TextPosition = PinCenter + FVector2D(-TextSize.X * 0.5f, -PinSize.Y * 0.5f - TextPadding - TextSize.Y);
		break;
	case EGraphDesignerPinTextPosition::Bottom:
		TextPosition = PinCenter + FVector2D(-TextSize.X * 0.5f, PinSize.Y * 0.5f + TextPadding);
		break;
	default:
		break;
	}

	DrawMultilineText(OutDrawElements, AllottedGeometry, LayerId, DisplayText, FontInfo, TextPosition, Pin.TextColor);
}

FString SGraphDesignerCanvas::NormalizeDisplayText(const FString& Text) const
{
	FString NormalizedText = Text;
	NormalizedText.ReplaceInline(TEXT("\\r\\n"), TEXT("\n"));
	NormalizedText.ReplaceInline(TEXT("\\n"), TEXT("\n"));
	NormalizedText.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
	NormalizedText.ReplaceInline(TEXT("\r"), TEXT("\n"));
	return NormalizedText;
}

FVector2D SGraphDesignerCanvas::MeasureMultilineText(const FString& Text, const FSlateFontInfo& FontInfo) const
{
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);
	if (Lines.Num() == 0)
	{
		Lines.Add(TEXT(""));
	}

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float LineHeight = FMath::Max(FontMeasure->Measure(FText::FromString(TEXT("Ag")), FontInfo).Y, 1.0f);
	FVector2D TextSize(0.0f, LineHeight * Lines.Num());
	for (const FString& Line : Lines)
	{
		TextSize.X = FMath::Max(TextSize.X, FontMeasure->Measure(FText::FromString(Line), FontInfo).X);
	}
	return TextSize;
}

void SGraphDesignerCanvas::DrawMultilineText(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FString& Text, const FSlateFontInfo& FontInfo, const FVector2D& TextPosition, const FLinearColor& TextColor) const
{
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);
	if (Lines.Num() == 0)
	{
		return;
	}

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float LineHeight = FMath::Max(FontMeasure->Measure(FText::FromString(TEXT("Ag")), FontInfo).Y, 1.0f);
	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		if (Lines[LineIndex].IsEmpty())
		{
			continue;
		}

		const FVector2D LinePosition = TextPosition + FVector2D(0.0f, LineHeight * LineIndex);
		const FVector2D LineSize = FontMeasure->Measure(FText::FromString(Lines[LineIndex]), FontInfo);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(LinePosition, LineSize),
			FText::FromString(Lines[LineIndex]),
			FontInfo,
			ESlateDrawEffect::None,
			TextColor
		);
	}
}

const FSlateBrush* SGraphDesignerCanvas::GetRoundedBoxBrush(const FLinearColor& FillColor, float Radius, const FLinearColor& OutlineColor, float OutlineWidth, const FVector2D& ImageSize) const
{
	const FString BrushKey = FString::Printf(
		TEXT("%.4f_%.4f_%.4f_%.4f_%.2f_%.4f_%.4f_%.4f_%.4f_%.2f_%.1f_%.1f"),
		FillColor.R,
		FillColor.G,
		FillColor.B,
		FillColor.A,
		Radius,
		OutlineColor.R,
		OutlineColor.G,
		OutlineColor.B,
		OutlineColor.A,
		OutlineWidth,
		ImageSize.X,
		ImageSize.Y
	);

	if (const TSharedPtr<FSlateBrush>* CachedBrush = ShapeBrushCache.Find(BrushKey))
	{
		return CachedBrush->Get();
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateRoundedBoxBrush>(FillColor, Radius, OutlineColor, OutlineWidth, FVector2f(ImageSize));
	ShapeBrushCache.Add(BrushKey, Brush);
	return Brush.Get();
}

void SGraphDesignerCanvas::DrawDashedPolyline(FSlateWindowElementList& OutDrawElements,
	const FGeometry& AllottedGeometry, int32 LayerId, const TArray<FVector2D>& Points, const FLinearColor& Color,
	float Thickness) const
{
	//TArray<FVector2D> DashPoints;
	float DashLength = 12.0f;
	float GapLength = 8.0f;
	for (int32 index = 0; index < Points.Num() - 1; index++)
	{
		FVector2D StartPoint = Points[index];
		FVector2D EndPoint = Points[index + 1];
		FVector2D Direction = (EndPoint - StartPoint).GetSafeNormal();
		float SegmentLength = (EndPoint - StartPoint).Length();
		float Distance = 0;
		
		while (Distance < SegmentLength)
		{
			FVector2D DashStart = StartPoint + Direction * Distance;
			float DashEndDistance = FMath::Min(Distance + DashLength, SegmentLength);
			FVector2D DashEnd = DashStart + Direction * DashEndDistance;
			
			FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			{DashStart, DashEnd},
			ESlateDrawEffect::None,
			Color,
			true,
			Thickness
			);
			
			Distance = Distance + DashLength + GapLength;
		
		}
	}
}

FGraphDesignerElement* SGraphDesignerCanvas::HitElement(const FVector2D& GraphPosition) const
{
	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return nullptr;
	}

	for (int32 Index = GraphDocument->Elements.Num() - 1; Index >= 0; --Index)
	{
		FGraphDesignerElement& Element = GraphDocument->Elements[Index];
		const FBox2D Bounds(Element.Position, Element.Position + Element.Size);
		if (Bounds.IsInside(GraphPosition))
		{
			return &Element;
		}
	}

	return nullptr;
}

FGraphDesignerPin* SGraphDesignerCanvas::HitPin(const FVector2D& GraphPosition, FGraphDesignerElement** OutOwnerElement) const
{
	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return nullptr;
	}

	const float HitRadius = GraphDesignerCanvas::PinHitRadius / Zoom;
	for (FGraphDesignerElement& Element : GraphDocument->Elements)
	{
		for (FGraphDesignerPin& Pin : Element.Pins)
		{
			const FVector2D PinGraphPosition = Element.Position + Pin.RelativePosition;
			if (FVector2D::Distance(PinGraphPosition, GraphPosition) <= HitRadius)
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

FGraphDesignerConnection* SGraphDesignerCanvas::HitConnection(const FVector2D& GraphPosition) const
{
	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return nullptr;
	}

	const float HitTolerance = GraphDesignerCanvas::ConnectionHitTolerance / Zoom;
	for (int32 Index = GraphDocument->Connections.Num() - 1; Index >= 0; --Index)
	{
		FGraphDesignerConnection& Connection = GraphDocument->Connections[Index];

		FVector2D StartGraph;
		FVector2D EndGraph;
		if (!GraphDocument->GetPinWorldPosition(Connection.StartPinId, StartGraph) || !GraphDocument->GetPinWorldPosition(Connection.EndPinId, EndGraph))
		{
			continue;
		}

		TArray<FVector2D> ConnectionGraphPoints;
		BuildConnectionGraphPoints(Connection, StartGraph, EndGraph, ConnectionGraphPoints);
		for (int32 PointIndex = 1; PointIndex < ConnectionGraphPoints.Num(); ++PointIndex)
		{
			if (DistancePointToSegment(GraphPosition, ConnectionGraphPoints[PointIndex - 1], ConnectionGraphPoints[PointIndex]) <= HitTolerance)
			{
				return &Connection;
			}
		}
	}

	return nullptr;
}

EGraphDesignerResizeHandle SGraphDesignerCanvas::HitResizeHandle(const FGraphDesignerElement& Element, const FVector2D& GraphPosition) const
{
	const float HandleGraphSize = GraphDesignerCanvas::ResizeHandleHitSize / Zoom;
	const FVector2D P = Element.Position;
	const FVector2D S = Element.Size;
	const TArray<TPair<EGraphDesignerResizeHandle, FVector2D>> Handles =
	{
		{EGraphDesignerResizeHandle::TopLeft, P},
		{EGraphDesignerResizeHandle::Top, P + FVector2D(S.X * 0.5f, 0.0f)},
		{EGraphDesignerResizeHandle::TopRight, P + FVector2D(S.X, 0.0f)},
		{EGraphDesignerResizeHandle::Right, P + FVector2D(S.X, S.Y * 0.5f)},
		{EGraphDesignerResizeHandle::BottomRight, P + S},
		{EGraphDesignerResizeHandle::Bottom, P + FVector2D(S.X * 0.5f, S.Y)},
		{EGraphDesignerResizeHandle::BottomLeft, P + FVector2D(0.0f, S.Y)},
		{EGraphDesignerResizeHandle::Left, P + FVector2D(0.0f, S.Y * 0.5f)}
	};

	for (const TPair<EGraphDesignerResizeHandle, FVector2D>& Handle : Handles)
	{
		if (FMath::Abs(GraphPosition.X - Handle.Value.X) <= HandleGraphSize && FMath::Abs(GraphPosition.Y - Handle.Value.Y) <= HandleGraphSize)
		{
			return Handle.Key;
		}
	}

	return EGraphDesignerResizeHandle::None;
}

void SGraphDesignerCanvas::SelectElement(FGraphDesignerElement& Element, bool bAppend)
{
	if (bGraphEditingLocked)
	{
		return;
	}

	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (!bAppend)
		{
			GraphDocument->ClearSelection();
		}
		Element.bSelected = true;
		BroadcastSelectionChanged();
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SGraphDesignerCanvas::SelectConnection(FGraphDesignerConnection& Connection, bool bAppend)
{
	if (bGraphEditingLocked)
	{
		return;
	}

	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (!bAppend)
		{
			GraphDocument->ClearSelection();
		}
		Connection.bSelected = true;
		BroadcastSelectionChanged();
		OnConnectionSelected.ExecuteIfBound(Connection.Id);
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SGraphDesignerCanvas::SelectElementsInMarquee()
{
	if (bGraphEditingLocked)
	{
		return;
	}

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	const FVector2D Min(FMath::Min(MarqueeStartGraphPosition.X, MarqueeEndGraphPosition.X), FMath::Min(MarqueeStartGraphPosition.Y, MarqueeEndGraphPosition.Y));
	const FVector2D Max(FMath::Max(MarqueeStartGraphPosition.X, MarqueeEndGraphPosition.X), FMath::Max(MarqueeStartGraphPosition.Y, MarqueeEndGraphPosition.Y));
	const FBox2D Marquee(Min, Max);

	for (FGraphDesignerElement& Element : GraphDocument->Elements)
	{
		Element.bSelected = Marquee.Intersect(FBox2D(Element.Position, Element.Position + Element.Size));
	}

	BroadcastSelectionChanged();
}

void SGraphDesignerCanvas::BroadcastSelectionChanged() const
{
	const UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	TArray<FString> SelectedIds;
	for (const FGraphDesignerElement& Element : GraphDocument->Elements)
	{
		if (Element.bSelected)
		{
			SelectedIds.Add(Element.Id);
		}
	}

	OnSelectionChanged.ExecuteIfBound(SelectedIds);
}

void SGraphDesignerCanvas::MoveSelectedElements(const FVector2D& GraphDelta)
{
	if (bGraphEditingLocked)
	{
		return;
	}

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	for (FGraphDesignerElement& Element : GraphDocument->Elements)
	{
		if (Element.bSelected)
		{
			Element.Position += GraphDelta;
			Element.bDragging = true;
		}
	}
}

void SGraphDesignerCanvas::ResizeElement(FGraphDesignerElement& Element, const FVector2D& GraphPosition)
{
	if (bGraphEditingLocked)
	{
		return;
	}

	FVector2D NewPosition = DragStartElementPosition;
	FVector2D NewSize = DragStartElementSize;
	const FVector2D Delta = GraphPosition - DragStartGraphPosition;

	if (ActiveResizeHandle == EGraphDesignerResizeHandle::Right || ActiveResizeHandle == EGraphDesignerResizeHandle::TopRight || ActiveResizeHandle == EGraphDesignerResizeHandle::BottomRight)
	{
		NewSize.X = FMath::Max(GraphDesignerCanvas::MinElementSize, DragStartElementSize.X + Delta.X);
	}
	if (ActiveResizeHandle == EGraphDesignerResizeHandle::Left || ActiveResizeHandle == EGraphDesignerResizeHandle::TopLeft || ActiveResizeHandle == EGraphDesignerResizeHandle::BottomLeft)
	{
		const float ProposedWidth = FMath::Max(GraphDesignerCanvas::MinElementSize, DragStartElementSize.X - Delta.X);
		NewPosition.X = DragStartElementPosition.X + (DragStartElementSize.X - ProposedWidth);
		NewSize.X = ProposedWidth;
	}
	if (ActiveResizeHandle == EGraphDesignerResizeHandle::Bottom || ActiveResizeHandle == EGraphDesignerResizeHandle::BottomLeft || ActiveResizeHandle == EGraphDesignerResizeHandle::BottomRight)
	{
		NewSize.Y = FMath::Max(GraphDesignerCanvas::MinElementSize, DragStartElementSize.Y + Delta.Y);
	}
	if (ActiveResizeHandle == EGraphDesignerResizeHandle::Top || ActiveResizeHandle == EGraphDesignerResizeHandle::TopLeft || ActiveResizeHandle == EGraphDesignerResizeHandle::TopRight)
	{
		const float ProposedHeight = FMath::Max(GraphDesignerCanvas::MinElementSize, DragStartElementSize.Y - Delta.Y);
		NewPosition.Y = DragStartElementPosition.Y + (DragStartElementSize.Y - ProposedHeight);
		NewSize.Y = ProposedHeight;
	}

	Element.Position = NewPosition;
	Element.Size = NewSize;
}

void SGraphDesignerCanvas::DeleteSelectedElements()
{
	if (bGraphEditingLocked)
	{
		return;
	}

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	TArray<FString> SelectedIds;
	for (const FGraphDesignerElement& Element : GraphDocument->Elements)
	{
		if (Element.bSelected)
		{
			SelectedIds.Add(Element.Id);
		}
	}

	if (GraphDocument->RemoveElements(SelectedIds) > 0)
	{
		BroadcastSelectionChanged();
		Invalidate(EInvalidateWidget::Paint);
	}
}

int32 SGraphDesignerCanvas::DeleteSelectedConnections()
{
	if (bGraphEditingLocked)
	{
		return 0;
	}

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return 0;
	}

	TArray<FString> SelectedConnectionIds;
	for (const FGraphDesignerConnection& Connection : GraphDocument->Connections)
	{
		if (Connection.bSelected)
		{
			SelectedConnectionIds.Add(Connection.Id);
		}
	}

	int32 RemovedCount = 0;
	for (const FString& ConnectionId : SelectedConnectionIds)
	{
		RemovedCount += GraphDocument->RemoveConnection(ConnectionId) ? 1 : 0;
	}

	if (RemovedCount > 0)
	{
		BroadcastSelectionChanged();
		Invalidate(EInvalidateWidget::Paint);
	}

	return RemovedCount;
}

void SGraphDesignerCanvas::CopySelectedElementsToClipboard()
{
	const UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	ClipboardElements.Reset();
	ClipboardConnections.Reset();
	PasteCount = 0;

	TSet<FString> SelectedElementIds;
	for (const FGraphDesignerElement& Element : GraphDocument->Elements)
	{
		if (Element.bSelected)
		{
			ClipboardElements.Add(Element);
			SelectedElementIds.Add(Element.Id);
		}
	}

	for (const FGraphDesignerConnection& Connection : GraphDocument->Connections)
	{
		const FGraphDesignerElement* StartOwnerElement = nullptr;
		const FGraphDesignerElement* EndOwnerElement = nullptr;
		GraphDocument->FindPin(Connection.StartPinId, &StartOwnerElement);
		GraphDocument->FindPin(Connection.EndPinId, &EndOwnerElement);

		const bool bBothEndsSelected = StartOwnerElement != nullptr
			&& EndOwnerElement != nullptr
			&& SelectedElementIds.Contains(StartOwnerElement->Id)
			&& SelectedElementIds.Contains(EndOwnerElement->Id);

		if (Connection.bSelected || bBothEndsSelected)
		{
			ClipboardConnections.Add(Connection);
		}
	}
}

void SGraphDesignerCanvas::PasteClipboardElements()
{
	if (bGraphEditingLocked || ClipboardElements.Num() == 0)
	{
		return;
	}

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return;
	}

	const FVector2D PasteOffset(32.0f * static_cast<float>(PasteCount + 1), 32.0f * static_cast<float>(PasteCount + 1));
	TMap<FString, FString> PinIdMap;

	GraphDocument->ClearSelection();

	for (const FGraphDesignerElement& SourceElement : ClipboardElements)
	{
		FGraphDesignerElement NewElement = SourceElement;
		NewElement.Id = GraphDesignerCanvas::MakeDuplicateId(TEXT("Element"));
		NewElement.Position += PasteOffset;
		NewElement.bSelected = true;
		NewElement.bDragging = false;

		for (FGraphDesignerPin& Pin : NewElement.Pins)
		{
			const FString OldPinId = Pin.Id;
			Pin.Id = GraphDesignerCanvas::MakeDuplicateId(TEXT("Pin"));
			Pin.OwnerElementId = NewElement.Id;
			Pin.bSelected = false;
			PinIdMap.Add(OldPinId, Pin.Id);
		}

		GraphDocument->Elements.Add(NewElement);
	}

	for (const FGraphDesignerConnection& SourceConnection : ClipboardConnections)
	{
		const FString* NewStartPinId = PinIdMap.Find(SourceConnection.StartPinId);
		const FString* NewEndPinId = PinIdMap.Find(SourceConnection.EndPinId);
		if (NewStartPinId == nullptr || NewEndPinId == nullptr)
		{
			continue;
		}

		FGraphDesignerConnection NewConnection = SourceConnection;
		NewConnection.Id = GraphDesignerCanvas::MakeDuplicateId(TEXT("Connection"));
		NewConnection.StartPinId = *NewStartPinId;
		NewConnection.EndPinId = *NewEndPinId;
		NewConnection.bSelected = false;
		for (FVector2D& RoutePoint : NewConnection.RoutePoints)
		{
			RoutePoint += PasteOffset;
		}
		GraphDocument->Connections.Add(NewConnection);
	}

	++PasteCount;
	BroadcastSelectionChanged();
	Invalidate(EInvalidateWidget::Paint);
}

FString SGraphDesignerCanvas::CreateElementFromTemplate(const FGraphDesignerNodeTemplate& NodeTemplate, const FVector2D& GraphPosition)
{
	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return TEXT("");
	}

	const FString ElementId = GraphDocument->AddElement(NodeTemplate.Type, GraphPosition, NodeTemplate.DefaultSize);
	if (FGraphDesignerElement* Element = GraphDocument->FindElement(ElementId))
	{
		Element->FillColor = NodeTemplate.FillColor;
		Element->BorderColor = NodeTemplate.BorderColor;
		Element->Text = NodeTemplate.Text.IsEmpty() ? NodeTemplate.DisplayName : NodeTemplate.Text;
		Element->TextColor = NodeTemplate.TextColor;
		Element->FontSize = NodeTemplate.FontSize;
		Element->ImagePath = NodeTemplate.ImagePath;
		Element->Pins.Reset();
	}

	for (const FGraphDesignerPinTemplate& PinTemplate : NodeTemplate.Pins)
	{
		const FString PinId = GraphDocument->AddPinToElement(ElementId, PinTemplate.RelativePosition, PinTemplate.Text);
		if (FGraphDesignerPin* Pin = GraphDocument->FindPin(PinId))
		{
			Pin->Size = PinTemplate.Size;
			Pin->FillColor = PinTemplate.FillColor;
			Pin->BorderColor = PinTemplate.BorderColor;
			Pin->Text = PinTemplate.Text;
			Pin->TextPosition = PinTemplate.TextPosition;
			Pin->TextColor = PinTemplate.TextColor;
			Pin->FontSize = PinTemplate.FontSize;
		}
	}

	GraphDocument->ClearSelection();
	GraphDocument->SetElementSelected(ElementId, true);
	BroadcastSelectionChanged();
	Invalidate(EInvalidateWidget::Paint);
	return ElementId;
}

FString SGraphDesignerCanvas::CreatePinFromTemplate(const FGraphDesignerNodeTemplate& NodeTemplate, const FVector2D& GraphPosition)
{
	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return TEXT("");
	}

	FVector2D RelativePosition;
	FGraphDesignerElement* OwnerElement = FindElementForPinDrop(GraphPosition, RelativePosition);
	if (OwnerElement == nullptr)
	{
		return TEXT("");
	}

	const FString PinId = GraphDocument->AddPinToElement(OwnerElement->Id, RelativePosition, NodeTemplate.Text);
	if (FGraphDesignerPin* Pin = GraphDocument->FindPin(PinId))
	{
		Pin->Size = NodeTemplate.DefaultSize;
		Pin->FillColor = NodeTemplate.FillColor;
		Pin->BorderColor = NodeTemplate.BorderColor;
		Pin->Text = NodeTemplate.Text;
		Pin->TextColor = NodeTemplate.TextColor;
		Pin->FontSize = NodeTemplate.FontSize;
	}
	/*if (OwnerElement->Type != EGraphDesignerElementType::Circle && OwnerElement->Type != EGraphDesignerElementType::Ellipse)
	{
		GraphDocument->ArrangePinsOnSameSide(OwnerElement->Id, RelativePosition);
	}*/

	GraphDocument->ClearSelection();
	OwnerElement->bSelected = true;
	BroadcastSelectionChanged();
	Invalidate(EInvalidateWidget::Paint);
	return PinId;
}

FGraphDesignerElement* SGraphDesignerCanvas::FindElementForPinDrop(const FVector2D& GraphPosition, FVector2D& OutRelativePosition) const
{
	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		return nullptr;
	}

	const float SnapDistance = GraphDesignerCanvas::PinDropSnapDistance / Zoom;
	for (int32 Index = GraphDocument->Elements.Num() - 1; Index >= 0; --Index)
	{
		FGraphDesignerElement& Element = GraphDocument->Elements[Index];
		if (Element.Type == EGraphDesignerElementType::Pin || Element.Type == EGraphDesignerElementType::Line || Element.Type == EGraphDesignerElementType::Arrow)
		{
			continue;
		}

		const FVector2D Min = Element.Position - FVector2D(SnapDistance, SnapDistance);
		const FVector2D Max = Element.Position + Element.Size + FVector2D(SnapDistance, SnapDistance);
		if (!FBox2D(Min, Max).IsInside(GraphPosition))
		{
			continue;
		}

		if (Element.Type == EGraphDesignerElementType::Circle || Element.Type == EGraphDesignerElementType::Ellipse)
		{
			const FVector2D LocalPosition = GraphPosition - Element.Position;
			const FVector2D Center = Element.Size * 0.5f;
			const FVector2D Radius = Element.Type == EGraphDesignerElementType::Circle
				? FVector2D(FMath::Min(Element.Size.X, Element.Size.Y) * 0.5f)
				: Element.Size * 0.5f;
			
			const FVector2D Offset = LocalPosition - Center;
			const float EllipseInsideValue = FMath::Square(Offset.X / Radius.X) + FMath::Square(Offset.Y / Radius.Y);
			const bool bInsideShape = (EllipseInsideValue <= 1.0f);
			
			FVector2D Direction = Offset;
			if (Direction.IsNearlyZero())
			{
				Direction = FVector2D(1.0f, 0.0f);
			}

			const float Denominator = FMath::Sqrt(FMath::Square(Direction.X / Radius.X) + FMath::Square(Direction.Y / Radius.Y));
			if (!FMath::IsNearlyZero(Denominator))
			{
				const FVector2D BoundaryPosition = Center + Direction / Denominator;
				const float DistanceToBoundary = FVector2D::Distance(LocalPosition, BoundaryPosition);
				if (FVector2D::Distance(LocalPosition, BoundaryPosition) <= SnapDistance)
				{
					OutRelativePosition = BoundaryPosition;
					return &Element;
				}
			}
			if (bInsideShape)
			{
				OutRelativePosition = LocalPosition;
				return &Element;
			}
			continue;
		}

		const float LeftDistance = FMath::Abs(GraphPosition.X - Element.Position.X);
		const float RightDistance = FMath::Abs(GraphPosition.X - (Element.Position.X + Element.Size.X));
		const float TopDistance = FMath::Abs(GraphPosition.Y - Element.Position.Y);
		const float BottomDistance = FMath::Abs(GraphPosition.Y - (Element.Position.Y + Element.Size.Y));
		float BestDistance = LeftDistance;
		OutRelativePosition = FVector2D(0.0f, FMath::Clamp(GraphPosition.Y - Element.Position.Y, 0.0f, Element.Size.Y));

		if (RightDistance < BestDistance)
		{
			BestDistance = RightDistance;
			OutRelativePosition = FVector2D(Element.Size.X, FMath::Clamp(GraphPosition.Y - Element.Position.Y, 0.0f, Element.Size.Y));
		}
		if (TopDistance < BestDistance)
		{
			BestDistance = TopDistance;
			OutRelativePosition = FVector2D(FMath::Clamp(GraphPosition.X - Element.Position.X, 0.0f, Element.Size.X), 0.0f);
		}
		if (BottomDistance < BestDistance)
		{
			BestDistance = BottomDistance;
			OutRelativePosition = FVector2D(FMath::Clamp(GraphPosition.X - Element.Position.X, 0.0f, Element.Size.X), Element.Size.Y);
		}

		const FVector2D LocalPosition = GraphPosition - Element.Position;
		
		const bool bInsideShape = LocalPosition.X >= 0.0f && LocalPosition.X <= Element.Size.X && LocalPosition.Y >= 0.0f && LocalPosition.Y <= Element.Size.Y;
		if (BestDistance <= SnapDistance)
		{
			return &Element;
		}
		
		if (bInsideShape)
		{
			OutRelativePosition = LocalPosition;
			return &Element;
		}
	}

	return nullptr;
}

void SGraphDesignerCanvas::DrawArrowHead(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness) const
{
	const FVector2D Direction = (End - Start).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FVector2D Normal(-Direction.Y, Direction.X);
	const float ArrowLength = 14.0f * Zoom;
	const FVector2D Left = End - Direction * ArrowLength + Normal * ArrowLength * 0.45f;
	const FVector2D Right = End - Direction * ArrowLength - Normal * ArrowLength * 0.45f;
	TArray<FVector2D> LeftLine = { End, Left };
	TArray<FVector2D> RightLine = { End, Right };
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), LeftLine, ESlateDrawEffect::None, Color, true, Thickness);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), RightLine, ESlateDrawEffect::None, Color, true, Thickness);
}

float SGraphDesignerCanvas::DistancePointToSegment(const FVector2D& Point, const FVector2D& SegmentStart, const FVector2D& SegmentEnd) const
{
	const FVector2D Segment = SegmentEnd - SegmentStart;
	const float SegmentLengthSquared = Segment.SizeSquared();
	if (FMath::IsNearlyZero(SegmentLengthSquared))
	{
		return FVector2D::Distance(Point, SegmentStart);
	}

	const float Alpha = FMath::Clamp(FVector2D::DotProduct(Point - SegmentStart, Segment) / SegmentLengthSquared, 0.0f, 1.0f);
	const FVector2D ClosestPoint = SegmentStart + Segment * Alpha;
	return FVector2D::Distance(Point, ClosestPoint);
}

FVector2D SGraphDesignerCanvas::GetElementCenter(const FGraphDesignerElement& Element) const
{
	return Element.Position + Element.Size * 0.5f;
}

FVector2D SGraphDesignerCanvas::RotatePointAround(const FVector2D& Point, const FVector2D& Center, float Degrees) const
{
	if (FMath::IsNearlyZero(Degrees))
	{
		return Point;
	}

	const float Radians = FMath::DegreesToRadians(Degrees);
	const float Cos = FMath::Cos(Radians);
	const float Sin = FMath::Sin(Radians);
	const FVector2D Local = Point - Center;
	return Center + FVector2D(Local.X * Cos - Local.Y * Sin, Local.X * Sin + Local.Y * Cos);
}

const FSlateBrush* SGraphDesignerCanvas::GetImageBrush(const FGraphDesignerElement& Element) const
{
	if (Element.ImagePath.IsEmpty())
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	const FString ResolvedImagePath = UGraphDocument::ResolveImagePath(Element.ImagePath);

	if (const TSharedPtr<FSlateBrush>* CachedBrush = ImageBrushCache.Find(ResolvedImagePath))
	{
		return CachedBrush->Get();
	}

	UTexture2D* Texture = nullptr;
	if (FPaths::FileExists(ResolvedImagePath) && IsRuntimeImageFile(ResolvedImagePath))
	{
		Texture = LoadTextureFromDisk(ResolvedImagePath);
	}
	else
	{
		Texture = LoadObject<UTexture2D>(nullptr, *ResolvedImagePath);
	}

	if (Texture == nullptr)
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->SetResourceObject(Texture);
	Brush->ImageSize = Element.Size;
	ImageBrushCache.Add(ResolvedImagePath, Brush);
	return Brush.Get();
}

bool SGraphDesignerCanvas::IsRuntimeImageFile(const FString& FilePath)
{
	const FString Extension = FPaths::GetExtension(FilePath).ToLower();
	return Extension == TEXT("png")
		|| Extension == TEXT("jpg")
		|| Extension == TEXT("jpeg")
		|| Extension == TEXT("bmp")
		|| Extension == TEXT("tga")
		|| Extension == TEXT("exr")
		|| Extension == TEXT("hdr");
}

UTexture2D* SGraphDesignerCanvas::LoadTextureFromDisk(const FString& FilePath) const
{
	if (UTexture2D** CachedTexture = ImageTextureCache.Find(FilePath))
	{
		return *CachedTexture;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath) || FileData.IsEmpty())
	{
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	const EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
	if (ImageFormat == EImageFormat::Invalid)
	{
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		return nullptr;
	}

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
	if (Texture == nullptr || Texture->GetPlatformData() == nullptr || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->AddToRoot();
	Texture->SRGB = true;
	Texture->NeverStream = true;
	Texture->MipGenSettings = TMGS_NoMipmaps;
	Texture->CompressionSettings = TextureCompressionSettings::TC_Default;

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	ImageTextureCache.Add(FilePath, Texture);
	return Texture;
}
