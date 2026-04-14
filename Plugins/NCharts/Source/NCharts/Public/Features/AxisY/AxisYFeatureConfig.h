#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "AxisYFeatureConfig.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API UAxisYFeatureConfig : public UNChartFeatureConfigBase
{
	GENERATED_BODY()

public:
	virtual EChartFeatureType GetFeatureType() const override;
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const override;
	
	// 坐标轴类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisY")
	EAxisType AxisType = EAxisType::Value;
	
	UPROPERTY()
	bool bIsCategoryMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NCharts|AxisY", meta = (EditCondition = "AxisType = EAxisType::Category", EditConditionHides))
	TArray<FString> AxisData ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisY")
	FLinearColor AxisColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisY")
	float AxisThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|AxisY")
	FVector2D Padding = FVector2D(16.0f, 12.0f);
	

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif
	
};
