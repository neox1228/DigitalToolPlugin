// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class FAxisXProxy;

/**
 * SAxisXWidget —— X 轴 Slate 绘制控件
 *
 * 继承 SLeafWidget，在 OnPaint 中自绘底部水平轴线。
 * 通过订阅 FAxisXProxy::OnStateChanged 实现状态驱动的增量重绘。
 */
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
	/** Proxy 状态变更回调，触发 Paint 失效 */
	void HandleStateChanged();

	TSharedPtr<FAxisXProxy> Proxy;
	FDelegateHandle StateChangedHandle;
};
