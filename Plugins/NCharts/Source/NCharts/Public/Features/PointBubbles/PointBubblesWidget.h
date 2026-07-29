// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Types/SlateEnums.h"
#include "Widgets/SLeafWidget.h"

class FActiveTimerHandle;
struct FPointerEvent;
class FPointBubblesProxy;

/**
 * SPointBubblesWidget —— 气泡点标注 Slate 交互控件
 *
 * 在数据点上绘制圆点，Hover 时触发呼吸动画并显示数值气泡。
 * 使用 RegisterActiveTimer 实现 Hover 状态下的持续动画刷新。
 */
class NCHARTS_API SPointBubblesWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SPointBubblesWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FPointBubblesProxy>, Proxy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SPointBubblesWidget() override;

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

	/** 呼吸动画定时器回调，持续触发 Paint 失效 */
	EActiveTimerReturnType HandleBreathTick(double InCurrentTime, float InDeltaTime);

	bool UpdateHoverState(const FGeometry& MyGeometry, const FVector2D& LocalMousePosition);
	void ClearHoverState();

	/** 将数据点映射为屏幕坐标 */
	bool BuildLayout(const FVector2D& GeometrySize, TArray<FVector2D>& OutScreenPoints) const;

	/** 使用同心圆环线段近似绘制实心圆 */
	void DrawCircle(
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FVector2D& Center,
		float Radius,
		const FLinearColor& Color) const;

	TSharedPtr<FPointBubblesProxy> Proxy;
	FDelegateHandle StateChangedHandle;
	TWeakPtr<FActiveTimerHandle> BreathTimerHandle;
};
