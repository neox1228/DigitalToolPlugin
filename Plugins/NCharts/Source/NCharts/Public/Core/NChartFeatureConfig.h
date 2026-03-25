#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NChartFeatureConfig.generated.h"

class INChartProxy;

UENUM(BlueprintType)
enum class EChartFeatureType : uint8
{
	AxisX UMETA(DisplayName = "Axis X"),
	AxisY UMETA(DisplayName = "Axis Y"),
	LineSeries UMETA(DisplayName = "Line Series"),
	Tooltip UMETA(DisplayName = "Tooltip")
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API UNChartFeatureConfigBase : public UObject
{
	GENERATED_BODY()

public:
	virtual EChartFeatureType GetFeatureType() const PURE_VIRTUAL(UNChartFeatureConfigBase::GetFeatureType, return EChartFeatureType::AxisX;);
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const {}
};
