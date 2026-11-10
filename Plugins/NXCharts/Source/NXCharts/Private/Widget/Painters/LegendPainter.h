#pragma once

#include "CoreMinimal.h"
#include "Widget/ChartPainter.h"
/** 图例。对应 Spec.Legend */
class FLegendPainter : public FChartPainter
{
public:
	virtual EChartLayer GetLayer() const override;
	virtual bool IsEnabled(const FChartPaintArgs& Args) const override;
	virtual void Paint(const FChartPaintArgs& Args) const override;
};