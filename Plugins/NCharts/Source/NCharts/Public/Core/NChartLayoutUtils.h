// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** 数据坐标到屏幕坐标的布局结果 */
struct NCHARTS_API FNChartScreenLayout
{
	FVector2D DrawMin = FVector2D::ZeroVector;
	FVector2D DrawMax = FVector2D::ZeroVector;
	FVector2D DataMin = FVector2D::ZeroVector;
	FVector2D DataMax = FVector2D::ZeroVector;
	TArray<FVector2D> ScreenPoints;
};

class NCHARTS_API FNChartLayoutUtils
{
public:
	static bool BuildScreenLayout(
		const TArray<FVector2D>& DataPoints,
		const FVector2D& Padding,
		const FVector2D& GeometrySize,
		int32 MinPointCount,
		FNChartScreenLayout& OutLayout);

	static bool ScreenXToDataX(float ScreenX, const FNChartScreenLayout& Layout, float& OutDataX);

	static int32 FindNearestByScreenPosition(
		const TArray<FVector2D>& ScreenPoints,
		const FVector2D& MousePosition,
		float& OutDistanceSquared);

	static int32 FindNearestByDataX(const TArray<FVector2D>& DataPoints, float DataX);
};
