#pragma once

#include "CoreMinimal.h"
#include "Widget/Zoom/ChartZoom.h"

/** 笛卡尔图表的缩放：以光标为锚点，同时缩 X/Y 两根轴，并按轴边界钳制 */
class FCartesianZoom : public IChartZoom
{
public:
	virtual bool HandleWheel(FChartZoomArgs& Args) override;
};