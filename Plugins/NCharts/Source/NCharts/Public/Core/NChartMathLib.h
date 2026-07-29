// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * NChartMathLib —— 图表数学工具库
 *
 * 提供图表数据处理相关的静态工具方法，
 * 目前主要用于折线/气泡点数据按 X 轴排序。
 */
class NChartMathLib
{
public:
	NChartMathLib();
	~NChartMathLib();

	/**
	 * 对二维点集按 X 坐标排序
	 * 折线图要求数据点沿 X 轴单调递增，绘制前需确保顺序正确
	 *
	 * @param InPoints  待排序的点集（原地修改）
	 * @param bAscending true = 升序（默认），false = 降序
	 */
	static void SortPointsByX(UPARAM(ref) TArray<FVector2D>& InPoints, bool bAscending = true);
};
