#pragma once

#include "CoreMinimal.h"
#include "Core/NXChartsTypes.h"

class FChartPaintContext;
class UNXChartWidget;
struct FChartFrame;

/** 单次绘制的输入。所有组件共用这一份上下文，组件之间不直接通信 */
struct FChartPaintArgs
{
	FChartPaintContext& Ctx;
	const FChartFrame& Frame;
	UNXChartWidget* Widget = nullptr;
	FVector2D LocalSize = FVector2D::ZeroVector;
	bool bHoverActive = false;
	FVector2D HoverLocal = FVector2D::ZeroVector;
};

/**
 * 绘制组件基类。
 *
 * 三条约定：
 *  1. 无状态：开关与样式一律从 Frame.Spec 读，保证 Spec 是唯一事实源
 *  2. 只读 Frame，组件之间不通信
 *  3. 层序由 GetLayer 决定，画布按层稳定排序后遍历
 *
 * 加一个新的绘制内容 = 新增一个子类 + 在画布注册一行，OnPaint 不需要改。
 */
class FChartPainter
{
public:
	virtual ~FChartPainter() = default;

	/** 绘制层。决定它画在谁上面 */
	virtual EChartLayer GetLayer() const = 0;

	/** 该不该画。默认全开，具体组件去读 Frame.Spec 里的开关 */
	virtual bool IsEnabled(const FChartPaintArgs& Args) const { return true; }

	/** 绘制。只读 Args，不得修改任何状态 */
	virtual void Paint(const FChartPaintArgs& Args) const = 0;
};