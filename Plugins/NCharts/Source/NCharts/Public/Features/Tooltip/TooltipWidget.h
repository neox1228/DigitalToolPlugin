// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SLeafWidget.h"

struct FPointerEvent;
class FTooltipProxy;

class NCHARTS_API STooltipWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(STooltipWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FTooltipProxy>, Proxy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~STooltipWidget() override;

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	void HandleStateChanged();
	bool UpdateHoverState(const FGeometry& MyGeometry, const FVector2D& LocalMousePosition);
	bool UpdateItemHoverState(const FVector2D& GeometrySize, const FVector2D& LocalMousePosition, float ActivationDistSq);
	bool UpdateAxisHoverState(const FVector2D& GeometrySize, const FVector2D& LocalMousePosition);
	void ClearHoverState();

	TSharedPtr<FTooltipProxy> Proxy;
	FDelegateHandle StateChangedHandle;
};
