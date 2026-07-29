// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "AxisXFeatureConfig.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API UAxisXFeatureConfig : public UNChartFeatureConfigBase
{
	GENERATED_BODY()

public:
	virtual EChartFeatureType GetFeatureType() const override;
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	EAxisType AxisType = EAxisType::Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX", meta = (EditCondition = "AxisType == EAxisType::Category", EditConditionHides))
	TArray<FString> CategoryData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX", meta = (EditCondition = "AxisType == EAxisType::Value", EditConditionHides))
	bool bAutoMin = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX", meta = (EditCondition = "AxisType == EAxisType::Value && !bAutoMin", EditConditionHides))
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX", meta = (EditCondition = "AxisType == EAxisType::Value", EditConditionHides))
	bool bAutoMax = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX", meta = (EditCondition = "AxisType == EAxisType::Value && !bAutoMax", EditConditionHides))
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX", meta = (ClampMin = "2", ClampMax = "12"))
	int32 SplitCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	bool bShowTicks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	bool bShowLabels = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	FLinearColor AxisColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	float AxisThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	FVector2D Padding = FVector2D(16.0f, 12.0f);
};
