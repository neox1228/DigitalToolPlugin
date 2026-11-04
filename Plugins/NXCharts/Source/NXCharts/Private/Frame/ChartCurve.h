// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 折线的曲线化。
 *
 * 关键约束：**必须在抽稀之后调用**。先按像素列把点数压到 O(宽度)，
 * 再做细分，否则 1 万点 × 每段 8 个采样 = 8 万个顶点，Slate 直接拖死。
 *
 * 平滑用的是 Fritsch–Carlson 单调三次插值：保证在相邻两点之间不过冲，
 * 数据剧烈抖动时不会冒出"数据里不存在"的尖峰——折线图里这比 Catmull-Rom 更安全。
 */
namespace ChartCurve
{
	/**
	 * 单调三次插值细分。
	 * @param In              已映射到局部坐标的顶点（X 需非递减，否则自动退化为折线）
	 * @param MaxOutputPoints 输出上限，超出的部分截断（防御性）
	 */
	NXCHARTS_API void SmoothMonotone(const TArray<FVector2d>& In, int32 MaxOutputPoints, TArray<FVector2d>& Out);

	/** 阶梯连线：每个区间先水平走到新 X，再竖直走到新 Y */
	NXCHARTS_API void Step(const TArray<FVector2d>& In, TArray<FVector2d>& Out);
}
