// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * UChartModel —— 图表数据模型（预留）
 *
 * 计划用于承载多条 Series 的渲染就绪数据（Render-ready series payload），
 * 将数据层与 Feature 绘制层解耦。当前仅为占位类，尚未实现具体逻辑。
 */
class NCHARTS_API UChartModel
{
	/** Series 名称标识 */
	FString Name;
};
