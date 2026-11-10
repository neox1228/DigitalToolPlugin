#pragma once

#include "CoreMinimal.h"
#include "Widget/ChartPainter.h"
/** 背景：纯色或垂直渐变。对应 Spec.Background */
class FBackgroundPainter : public FChartPainter
{
public:
	virtual EChartLayer GetLayer() const override;
	virtual void Paint(const FChartPaintArgs& Args) const override;
};