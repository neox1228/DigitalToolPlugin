// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"

struct NCHARTS_API FNChartAxisTick
{
	float DataValue = 0.0f;
	int32 CategoryIndex = INDEX_NONE;
	FText Label;
	float ScreenPos = 0.0f;
};

struct NCHARTS_API FNChartAxisDomain
{
	EAxisType Type = EAxisType::Value;
	TArray<FString> Categories;
	float Min = 0.0f;
	float Max = 1.0f;
	bool bAutoMin = true;
	bool bAutoMax = true;
	int32 SplitCount = 5;
};

class NCHARTS_API FNChartCartesianScale : public TSharedFromThis<FNChartCartesianScale>
{
public:
	FNChartAxisDomain XDomain;
	FNChartAxisDomain YDomain;
	FVector2D Padding = FVector2D(16.0f, 12.0f);
	FVector2D DrawMin = FVector2D::ZeroVector;
	FVector2D DrawMax = FVector2D::ZeroVector;
	TArray<FNChartAxisTick> XTicks;
	TArray<FNChartAxisTick> YTicks;

	void UpdatePixelRect(const FVector2D& InPadding, const FVector2D& GeometrySize);
	void BuildTicks();

	FVector2D DataToScreen(const FVector2D& DataPoint) const;
	FVector2D ScreenToData(const FVector2D& ScreenPoint) const;
	float DataXToScreen(float DataX) const;
	float DataYToScreen(float DataY) const;
	bool ScreenXToDataX(float ScreenX, float& OutDataX) const;
	bool ScreenYToDataY(float ScreenY, float& OutDataY) const;

	void BuildScreenPoints(const TArray<FVector2D>& DataPoints, TArray<FVector2D>& OutScreenPoints) const;

	static void CalcNiceScale(float InMin, float InMax, int32 MaxTicks, float& OutMin, float& OutMax, float& OutStep);
	static void BuildValueDomainFromExtent(float ExtentMin, float ExtentMax, FNChartAxisDomain& InOutDomain);
};
