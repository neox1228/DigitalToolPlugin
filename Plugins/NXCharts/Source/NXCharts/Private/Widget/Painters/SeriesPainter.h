#pragma once

#include "CoreMinimal.h"
#include "Widget/ChartPainter.h"
/** 曲线：面积填充 → 发光 → 主曲线 → 数据点。对应 Spec.Series */
class FSeriesPainter : public FChartPainter
{
public:
	virtual EChartLayer GetLayer() const override;
	virtual bool IsEnabled(const FChartPaintArgs& Args) const override;
	virtual void Paint(const FChartPaintArgs& Args) const override;
};