#pragma once

#include "CoreMinimal.h"
#include "Widget/ChartPainter.h"
/**
 * 网格 + 轴线 + 刻度标签 + 轴标题 + 图表标题。
 * 对应 Spec.Grid / Spec.AxisX / Spec.AxisY / Spec.Title
 *
 * 目前是合并实现，后续可拆成 Title / Grid / Axis 三个组件。
 */
class FGridAndAxesPainter : public FChartPainter
{
public:
	virtual EChartLayer GetLayer() const override;
	virtual void Paint(const FChartPaintArgs& Args) const override;
};