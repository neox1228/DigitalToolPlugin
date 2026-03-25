#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "TooltipFeatureConfig.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API UTooltipFeatureConfig : public UNChartFeatureConfigBase
{
	GENERATED_BODY()

public:
	virtual EChartFeatureType GetFeatureType() const override;
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	bool bEnableTooltip = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	bool bShowHoverLine = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	bool bShowHoverPoint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	float ActivationDistance = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	float HoverPointRadius = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	FLinearColor HoverLineColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	FLinearColor HoverPointColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	FLinearColor TooltipBackgroundColor = FLinearColor(0.08f, 0.08f, 0.08f, 0.90f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	FLinearColor TooltipTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|Tooltip")
	FVector2D TooltipOffset = FVector2D(12.0f, -12.0f);
};
