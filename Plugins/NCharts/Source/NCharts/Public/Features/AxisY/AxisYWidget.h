#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class FAxisYProxy;

class NCHARTS_API SAxisYWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SAxisYWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FAxisYProxy>, Proxy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SAxisYWidget() override;

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

	TSharedPtr<FAxisYProxy> Proxy;
	FDelegateHandle StateChangedHandle;
};
