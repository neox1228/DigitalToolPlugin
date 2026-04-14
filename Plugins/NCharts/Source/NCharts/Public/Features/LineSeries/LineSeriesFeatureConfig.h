#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "LineSeriesFeatureConfig.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API ULineSeriesFeatureConfig : public UNChartFeatureConfigBase
{
	GENERATED_BODY()

public:
	ULineSeriesFeatureConfig();

	virtual EChartFeatureType GetFeatureType() const override;
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|LineSeries")
	FLinearColor LineColor = FLinearColor(0.15f, 0.65f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|LineSeries")
	TArray<FVector2D> Points;
	//TMap<FString, TArray<FVector2D>> Points;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|LineSeries")
	FVector2D Padding = FVector2D(16.0f, 12.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|LineSeries")
	bool bUseLimit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|LineSeries")
	float YLimit = 25.0f;
};
