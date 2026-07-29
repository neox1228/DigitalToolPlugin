// Copyright NCharts Plugin. All Rights Reserved.

#include "Core/NChartCartesianScaleBuilder.h"

#include "Core/NChartTooltipTypes.h"
#include "Core/NChartTypes.h"
#include "Features/AxisX/AxisXFeatureConfig.h"
#include "Features/AxisY/AxisYFeatureConfig.h"

void FNChartCartesianScaleBuilder::CollectDataPoints(
	const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies,
	TArray<FVector2D>& OutPoints)
{
	OutPoints.Reset();

	for (const TPair<EChartFeatureType, TSharedPtr<INChartProxy>>& Pair : FeatureProxies)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		if (INChartTooltipDataProvider* Provider = Pair.Value->GetTooltipDataProvider())
		{
			OutPoints.Append(Provider->GetDataPoints());
		}
	}
}

FNChartAxisDomain FNChartCartesianScaleBuilder::MakeDomainFromConfig(
	EAxisType Type,
	const TArray<FString>& CategoryData,
	bool bAutoMin,
	bool bAutoMax,
	float MinValue,
	float MaxValue,
	int32 SplitCount,
	float ExtentMin,
	float ExtentMax,
	bool bIsXAxis)
{
	FNChartAxisDomain Domain;
	Domain.Type = Type;
	Domain.Categories = CategoryData;
	Domain.bAutoMin = bAutoMin;
	Domain.bAutoMax = bAutoMax;
	Domain.Min = MinValue;
	Domain.Max = MaxValue;
	Domain.SplitCount = FMath::Max(SplitCount, 2);

	if (Type == EAxisType::Category)
	{
		if (Domain.Categories.Num() == 0)
		{
			const int32 Count = FMath::Max(FMath::CeilToInt(ExtentMax) + 1, 1);
			for (int32 Index = 0; Index < Count; ++Index)
			{
				Domain.Categories.Add(FString::Printf(TEXT("%s%d"), bIsXAxis ? TEXT("X") : TEXT("Y"), Index));
			}
		}
		return Domain;
	}

	const float SafeMin = ExtentMin;
	const float SafeMax = ExtentMax > ExtentMin ? ExtentMax : ExtentMin + 1.0f;
	FNChartCartesianScale::BuildValueDomainFromExtent(SafeMin, SafeMax, Domain);
	return Domain;
}

TSharedRef<FNChartCartesianScale> FNChartCartesianScaleBuilder::Build(
	const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies,
	const UAxisXFeatureConfig* AxisXConfig,
	const UAxisYFeatureConfig* AxisYConfig)
{
	TArray<FVector2D> AllPoints;
	CollectDataPoints(FeatureProxies, AllPoints);

	float XMin = FLT_MAX;
	float XMax = -FLT_MAX;
	float YMin = FLT_MAX;
	float YMax = -FLT_MAX;
	for (const FVector2D& Point : AllPoints)
	{
		XMin = FMath::Min(XMin, Point.X);
		XMax = FMath::Max(XMax, Point.X);
		YMin = FMath::Min(YMin, Point.Y);
		YMax = FMath::Max(YMax, Point.Y);
	}

	if (AllPoints.Num() == 0)
	{
		XMin = 0.0f;
		XMax = 1.0f;
		YMin = 0.0f;
		YMax = 1.0f;
	}

	TSharedRef<FNChartCartesianScale> Scale = MakeShared<FNChartCartesianScale>();

	const EAxisType XType = AxisXConfig ? AxisXConfig->AxisType : EAxisType::Value;
	const EAxisType YType = AxisYConfig ? AxisYConfig->AxisType : EAxisType::Value;

	Scale->XDomain = MakeDomainFromConfig(
		XType,
		AxisXConfig ? AxisXConfig->CategoryData : TArray<FString>(),
		AxisXConfig ? AxisXConfig->bAutoMin : true,
		AxisXConfig ? AxisXConfig->bAutoMax : true,
		AxisXConfig ? AxisXConfig->MinValue : 0.0f,
		AxisXConfig ? AxisXConfig->MaxValue : 1.0f,
		AxisXConfig ? AxisXConfig->SplitCount : 5,
		XMin,
		XMax,
		true);

	Scale->YDomain = MakeDomainFromConfig(
		YType,
		AxisYConfig ? AxisYConfig->CategoryData : TArray<FString>(),
		AxisYConfig ? AxisYConfig->bAutoMin : true,
		AxisYConfig ? AxisYConfig->bAutoMax : true,
		AxisYConfig ? AxisYConfig->MinValue : 0.0f,
		AxisYConfig ? AxisYConfig->MaxValue : 1.0f,
		AxisYConfig ? AxisYConfig->SplitCount : 5,
		YMin,
		YMax,
		false);

	if (AxisXConfig)
	{
		Scale->Padding.X = AxisXConfig->Padding.X;
	}
	if (AxisYConfig)
	{
		Scale->Padding.Y = AxisYConfig->Padding.Y;
	}

	return Scale;
}
