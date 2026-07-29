#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class UChartModule;
class UChartComponent;
class UChartWidget;


class SChartCanvas : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SChartCanvas) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UChartWidget* InOwner);

	// 绘制入口
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
						  const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
						  int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
	virtual FVector2D ComputeDesiredSize(float) const override;

	// 交互事件
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	// 弱引用，避免循环引用
	TWeakObjectPtr<UChartWidget> Owner;

	// 当前悬停信息（为了示例，简单记录）
	mutable int32 LastHitComponentIndex = -1;
	mutable int32 LastHitItemIndex = -1;
};