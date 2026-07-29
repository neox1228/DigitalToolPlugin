#pragma once

#include "CoreMinimal.h"
#include "GraphDocument.h"
#include "Input/Reply.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_TwoParams(FOnGraphDesignerConnectionCreated, const FString& /*StartPinId*/, const FString& /*EndPinId*/)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerSelectionChanged, const TArray<FString>& /*SelectedElementIds*/)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerPinSelected, const FString& /*PinId*/)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerConnectionSelected, const FString& /*ConnectionId*/)

class UTexture2D;

class GRAPHDESIGNER_API SGraphDesignerCanvas : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGraphDesignerCanvas)
		: _Document(nullptr)
		, _BackgroundColor(FLinearColor(0.04f, 0.045f, 0.055f, 1.0f))
		, _GridColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.08f))
		, _SelectionColor(FLinearColor(0.1f, 0.45f, 1.0f, 1.0f))
		, _bGraphEditingLocked(false)
		, _ConnectionRoutingMode(EGraphDesignerConnectionRoutingMode::AutoOrthogonal)
	{
	}
		SLATE_ARGUMENT(UGraphDocument*, Document)
		SLATE_ATTRIBUTE(FLinearColor, BackgroundColor)
		SLATE_ATTRIBUTE(FLinearColor, GridColor)
		SLATE_ATTRIBUTE(FLinearColor, SelectionColor)
		SLATE_ARGUMENT(bool, bGraphEditingLocked)
		SLATE_ARGUMENT(EGraphDesignerConnectionRoutingMode, ConnectionRoutingMode)
		SLATE_EVENT(FOnGraphDesignerConnectionCreated, OnConnectionCreated)
		SLATE_EVENT(FOnGraphDesignerSelectionChanged, OnSelectionChanged)
		SLATE_EVENT(FOnGraphDesignerPinSelected, OnPinSelected)
		SLATE_EVENT(FOnGraphDesignerConnectionSelected, OnConnectionSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SGraphDesignerCanvas();

	void SetDocument(UGraphDocument* InDocument);
	UGraphDocument* GetDocument() const { return Document.Get(); }

	void SetZoom(float InZoom);
	float GetZoom() const { return Zoom; }

	void SetCanvasPan(FVector2D InPan);
	FVector2D GetCanvasPan() const { return CanvasPan; }

	void SetGraphEditingLocked(bool bLocked);
	bool IsGraphEditingLocked() const { return bGraphEditingLocked; }

	void SetConnectionRoutingMode(EGraphDesignerConnectionRoutingMode InRoutingMode);
	EGraphDesignerConnectionRoutingMode GetConnectionRoutingMode() const { return ConnectionRoutingMode; }

	void ClearSelection();

	FVector2D GraphToScreen(const FVector2D& GraphPosition) const;
	FVector2D ScreenToGraph(const FVector2D& LocalPosition) const;

protected:
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled
	) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }

private:
	enum class EInteractionMode
	{
		None,
		Panning,
		DraggingElements,
		ResizingElement,
		CreatingConnection,
		MarqueeSelecting
	};

	void DrawBackground(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void DrawGrid(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void DrawConnections(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void DrawElements(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void DrawElement(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, int32 LayerId) const;
	void DrawAllPins(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void DrawPins(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, int32 LayerId) const;
	void DrawConnectionPreview(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void DrawSelectionAndHandles(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, int32 LayerId) const;
	void DrawMarquee(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	void BuildOrthogonalConnectionPoints(const FVector2D& Start, const FVector2D& End, TArray<FVector2D>& OutPoints) const;
	void BuildConnectionGraphPoints(const FGraphDesignerConnection& Connection, const FVector2D& StartGraph, const FVector2D& EndGraph, TArray<FVector2D>& OutPoints) const;
	void BuildManualPreviewGraphPoints(const FVector2D& StartGraph, const FVector2D& EndGraph, TArray<FVector2D>& OutPoints) const;
	void ResetPendingConnection();
	void DrawFilledPolygon(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const TArray<FVector2D>& Points, const FLinearColor& FillColor) const;
	void DrawFilledEllipse(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Center, const FVector2D& Radius, const FLinearColor& FillColor) const;
	void DrawEllipseOutline(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Center, const FVector2D& Radius, const FLinearColor& Color, float Thickness) const;
	bool BuildElementPolygonPoints(EGraphDesignerElementType Type, const FVector2D& Position, const FVector2D& Size, TArray<FVector2D>& OutPoints) const;
	void BuildRoundedRectanglePoints(const FVector2D& Position, const FVector2D& Size, float Radius, TArray<FVector2D>& OutPoints) const;
	void DrawElementText(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerElement& Element, const FVector2D& ScreenPosition, const FVector2D& ScreenSize, int32 LayerId) const;
	void DrawPinText(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const FGraphDesignerPin& Pin, const FVector2D& PinCenter, const FVector2D& PinSize, int32 LayerId) const;
	FString NormalizeDisplayText(const FString& Text) const;
	FVector2D MeasureMultilineText(const FString& Text, const FSlateFontInfo& FontInfo) const;
	void DrawMultilineText(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FString& Text, const FSlateFontInfo& FontInfo, const FVector2D& TextPosition, const FLinearColor& TextColor) const;
	const FSlateBrush* GetRoundedBoxBrush(const FLinearColor& FillColor, float Radius, const FLinearColor& OutlineColor, float OutlineWidth, const FVector2D& ImageSize) const;
	void DrawDashedPolyline(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const TArray<FVector2D>& Points, const FLinearColor& Color, float Thickness) const;

	FGraphDesignerElement* HitElement(const FVector2D& GraphPosition) const;
	FGraphDesignerPin* HitPin(const FVector2D& GraphPosition, FGraphDesignerElement** OutOwnerElement = nullptr) const;
	FGraphDesignerConnection* HitConnection(const FVector2D& GraphPosition) const;
	EGraphDesignerResizeHandle HitResizeHandle(const FGraphDesignerElement& Element, const FVector2D& GraphPosition) const;

	void SelectElement(FGraphDesignerElement& Element, bool bAppend);
	void SelectConnection(FGraphDesignerConnection& Connection, bool bAppend);
	void SelectElementsInMarquee();
	void BroadcastSelectionChanged() const;
	void MoveSelectedElements(const FVector2D& GraphDelta);
	void ResizeElement(FGraphDesignerElement& Element, const FVector2D& GraphPosition);
	void DeleteSelectedElements();
	int32 DeleteSelectedConnections();
	void CopySelectedElementsToClipboard();
	void PasteClipboardElements();
	FString CreateElementFromTemplate(const FGraphDesignerNodeTemplate& NodeTemplate, const FVector2D& GraphPosition);
	FString CreatePinFromTemplate(const FGraphDesignerNodeTemplate& NodeTemplate, const FVector2D& GraphPosition);
	FGraphDesignerElement* FindElementForPinDrop(const FVector2D& GraphPosition, FVector2D& OutRelativePosition) const;
	void DrawArrowHead(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness) const;
	float DistancePointToSegment(const FVector2D& Point, const FVector2D& SegmentStart, const FVector2D& SegmentEnd) const;
	FVector2D GetElementCenter(const FGraphDesignerElement& Element) const;
	FVector2D RotatePointAround(const FVector2D& Point, const FVector2D& Center, float Degrees) const;
	const FSlateBrush* GetImageBrush(const FGraphDesignerElement& Element) const;
	UTexture2D* LoadTextureFromDisk(const FString& FilePath) const;
	static bool IsRuntimeImageFile(const FString& FilePath);

	TWeakObjectPtr<UGraphDocument> Document;
	FOnGraphDesignerConnectionCreated OnConnectionCreated;
	FOnGraphDesignerSelectionChanged OnSelectionChanged;
	FOnGraphDesignerPinSelected OnPinSelected;
	FOnGraphDesignerConnectionSelected OnConnectionSelected;

	FLinearColor BackgroundColor;
	FLinearColor GridColor;
	FLinearColor SelectionColor;
	bool bGraphEditingLocked = false;
	EGraphDesignerConnectionRoutingMode ConnectionRoutingMode = EGraphDesignerConnectionRoutingMode::AutoOrthogonal;
	float Zoom = 1.0f;
	FVector2D CanvasPan = FVector2D::ZeroVector;
	FVector2D LastMouseLocalPosition = FVector2D::ZeroVector;
	FVector2D LastMouseGraphPosition = FVector2D::ZeroVector;
	FVector2D DragStartGraphPosition = FVector2D::ZeroVector;
	FVector2D DragStartElementPosition = FVector2D::ZeroVector;
	FVector2D DragStartElementSize = FVector2D::ZeroVector;
	FVector2D MarqueeStartGraphPosition = FVector2D::ZeroVector;
	FVector2D MarqueeEndGraphPosition = FVector2D::ZeroVector;
	EInteractionMode InteractionMode = EInteractionMode::None;
	EGraphDesignerResizeHandle ActiveResizeHandle = EGraphDesignerResizeHandle::None;
	FString ActiveElementId;
	FString PendingConnectionStartPinId;
	TArray<FVector2D> PendingRoutePoints;
	TArray<FGraphDesignerElement> ClipboardElements;
	TArray<FGraphDesignerConnection> ClipboardConnections;
	int32 PasteCount = 0;
	mutable TMap<FString, TSharedPtr<FSlateBrush>> ImageBrushCache;
	mutable TMap<FString, UTexture2D*> ImageTextureCache;
	mutable TMap<FString, TSharedPtr<FSlateBrush>> ShapeBrushCache;
};
