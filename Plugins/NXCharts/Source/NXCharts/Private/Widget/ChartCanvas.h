#pragma once

#include "CoreMinimal.h"
#include "Layout/ChartSpace.h"
#include "Widget/ChartPainter.h"
#include "Widget/Zoom/ChartZoom.h"
#include "Widgets/SLeafWidget.h"

class UNXChartWidget;

/**
 * 图表的 Slate 画布。
 *
 * 它是纯绘制层：OnPaint 只读 UNXChartWidget 提供的不可变帧快照（FChartFrame），
 * 不做布局、不做坐标映射、不创建对象。
 */
class SChartCanvas : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SChartCanvas) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UNXChartWidget* InOwner);

	// 绘制入口
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                      const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	                      int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;

	// 交互事件（悬停/提示框在后续批次接入，这里先保留入口）
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	/** 绘制组件。按 GetLayer 稳定排序后遍历，OnPaint 不再认识任何具体绘制内容 */
	TArray<TUniquePtr<FChartPainter>> Painters;

	/** 缩放策略。画布只调用，不认识具体算法 */
	TUniquePtr<IChartZoom> Zoom;



	// 弱引用，避免循环引用
	TWeakObjectPtr<UNXChartWidget> Owner;

	/** 上次绘制时的控件尺寸，用于调试与后续增量优化 */
	mutable FVector2D LastPaintSize = FVector2D::ZeroVector;

	/** 悬停状态：只在绘图区内有效 */
	mutable bool bHoverActive = false;
	mutable FVector2D HoverLocal = FVector2D::ZeroVector;

	/** 上次广播给蓝图的悬停目标，避免每次鼠标移动都刷事件 */
	mutable FName LastHoverSeriesKey;
	mutable int32 LastHoverPointIndex = INDEX_NONE;

	/** 中键拖拽平移的状态（拖拽期间以按下瞬间的坐标为基准） */
	bool bPanning = false;

	/** 配置文件轮询计时 */
	float SpecCheckAccumulator = 0.0f;
	FVector2D PanStartLocal = FVector2D::ZeroVector;
	FChartCartesianSpace PanStartSpace;

	/** 双击检测：上次左键按下的时间与位置 */
	mutable double LastClickTime = 0.0;
	mutable FVector2D LastClickPosition = FVector2D::ZeroVector;
};
