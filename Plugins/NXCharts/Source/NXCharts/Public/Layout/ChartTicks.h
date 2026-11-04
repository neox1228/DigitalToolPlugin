// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartTicks.h
 * @brief   刻度生成与数值格式化（1-2-5 x 10^n 的 nice numbers）。
 *
 * 为什么需要它：如果直接用「跨度 / 刻度数」当步长，会得到 0.037 这种
 * 人读不懂的刻度。nice-number 保证步长永远是 1、2、5、10、20、50 这类值。
 *
 * 纯 C++，不依赖 Slate，可以直接单测（见 Private/Tests/ChartLayoutTests.cpp）。
 */

#pragma once

#include "CoreMinimal.h"
#include "Core/NXChartsTypes.h"
#include "Layout/ChartSpace.h"

/** 单个刻度 */
struct NXCHARTS_API FChartTick
{
	/** 数据值（数据空间，不是像素） */
	double Value = 0.0;

	/** 显示文本；次刻度为空 */
	FString Label;

	/** 主/次刻度。绘制时只有主刻度带标签 */
	bool bMajor = true;
};

/** 一根轴上的全部刻度。主次放在同一个数组里，顺序即绘制顺序 */
struct NXCHARTS_API FChartTickSet
{
	TArray<FChartTick> Ticks;

	/** 主刻度步长（nice step），tooltip 的数值精度也复用它 */
	double Step = 0.0;

	void Reset();
	bool IsEmpty() const { return Ticks.Num() == 0; }
	int32 NumMajor() const;
	int32 NumMinor() const;
};

namespace ChartTicks
{
	/** 在给定跨度与期望间隔数下选出一个 nice 步长 */
	NXCHARTS_API double NiceStep(double Range, int32 DesiredIntervals);

	/** 由步长推导需要显示的小数位数（step=0.05 得到 2 位） */
	NXCHARTS_API int32 DecimalsForStep(double Step);

	/** 按格式把刻度值转成字符串。Auto 会依据 Step 自动决定小数位 */
	NXCHARTS_API FString FormatValue(double Value, double Step, EChartTickFormat Format);

	/**
	 * 生成线性刻度。
	 * @param DesiredTickCount 期望的主刻度数量（含首尾），实际数量由 nice step 决定
	 * @param bIncludeMinor    是否插入 5 等分的次刻度（仅当主刻度数量不太多时）
	 */
	NXCHARTS_API FChartTickSet MakeLinearTicks(const FChartDomain& Domain, int32 DesiredTickCount,
	                                           EChartTickFormat Format, bool bIncludeMinor = true);
}