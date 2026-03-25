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
	FLinearColor AxisColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	float AxisThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisX")
	FVector2D Padding = FVector2D(16.0f, 12.0f);
};
