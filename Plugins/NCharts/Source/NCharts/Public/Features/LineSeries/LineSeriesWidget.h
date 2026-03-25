#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class FLineSeriesProxy;

class NCHARTS_API SLineSeriesWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SLineSeriesWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FLineSeriesProxy>, Proxy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SLineSeriesWidget() override;

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	void HandleStateChanged();

	TSharedPtr<FLineSeriesProxy> Proxy;
	FDelegateHandle StateChangedHandle;
};
