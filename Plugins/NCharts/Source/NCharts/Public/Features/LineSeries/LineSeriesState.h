// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * FLineSeriesState —— 折线序列绘制状态
 *
 * 包含数据点、颜色、超限检测及绘制边距等全部渲染所需信息。
 */
struct NCHARTS_API FLineSeriesState
{
	/** 折线数据点集，X/Y 为数据坐标（非屏幕坐标） */
	TArray<FVector2D> Points;

	/** 正常状态下的折线颜色 */
	FLinearColor BaseLineColor = FLinearColor(0.15f, 0.65f, 1.0f, 1.0f);

	/** 数据超限时使用的警告颜色 */
	FLinearColor OverLimitColor = FLinearColor(1.0f, 0.35f, 0.2f, 1.0f);

	/** 当前实际使用的折线颜色（由 UpdateDerivedState 根据超限状态决定） */
	FLinearColor ActiveLineColor = BaseLineColor;

	/** 折线粗细（像素） */
	float LineThickness = 2.0f;

	/** 绘制区域内边距 */
	FVector2D Padding = FVector2D(16.0f, 12.0f);

	/** 是否存在任意数据点超过 YLimit */
	bool bOverLimit = false;

	/** 是否启用 Y 轴上限检测 */
	bool bUseLimit = false;

	/** Y 轴上限阈值 */
	float YLimit = 0.0f;
};
