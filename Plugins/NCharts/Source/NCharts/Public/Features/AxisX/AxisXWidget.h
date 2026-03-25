#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class FAxisXProxy;

class NCHARTS_API SAxisXWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SAxisXWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FAxisXProxy>, Proxy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SAxisXWidget() override;

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

	TSharedPtr<FAxisXProxy> Proxy;
	FDelegateHandle StateChangedHandle;
};
