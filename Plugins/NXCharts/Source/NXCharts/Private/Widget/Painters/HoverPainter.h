#pragma once

#include "CoreMinimal.h"
#include "Widget/ChartPainter.h"
/** 十字准线 + 提示框（鼠标悬停或外部准星）。对应 Spec.Colors */
class FHoverPainter : public FChartPainter
{
public:
	virtual EChartLayer GetLayer() const override;
	virtual bool IsEnabled(const FChartPaintArgs& Args) const override;
	virtual void Paint(const FChartPaintArgs& Args) const override;
};