// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class FLineSeriesProxy;

/**
 * SLineSeriesWidget —— 折线 Slate 绘制控件
 *
 * 核心绘制逻辑：
 * 1. 计算数据点的 min/max 范围
 * 2. 将数据坐标归一化映射到屏幕绘制区域
 * 3. 使用 FSlateDrawElement::MakeLines 绘制折线
 */
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
