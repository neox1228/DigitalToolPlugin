// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartCartesianScale.h"

class INChartProxy;
class INChartTooltipDataProvider;
class UAxisXFeatureConfig;
class UAxisYFeatureConfig;

class NCHARTS_API FNChartCartesianScaleBuilder
{
public:
	static TSharedRef<FNChartCartesianScale> Build(
		const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies,
		const UAxisXFeatureConfig* AxisXConfig,
		const UAxisYFeatureConfig* AxisYConfig);

private:
	static void CollectDataPoints(
		const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies,
		TArray<FVector2D>& OutPoints);

	static FNChartAxisDomain MakeDomainFromConfig(
		EAxisType Type,
		const TArray<FString>& CategoryData,
		bool bAutoMin,
		bool bAutoMax,
		float MinValue,
		float MaxValue,
		int32 SplitCount,
		float ExtentMin,
		float ExtentMax,
		bool bIsXAxis);
};
