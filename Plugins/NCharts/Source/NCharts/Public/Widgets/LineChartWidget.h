#pragma once

#include "Core/NChartFeatureConfig.h"
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "LineChartWidget.generated.h"

class FAxisXProxy;
class FAxisYProxy;
class FLineSeriesProxy;
class FTooltipProxy;
struct FPropertyChangedEvent;
struct FPropertyChangedChainEvent;

USTRUCT(BlueprintType)
struct FNChartFeatureEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NCharts|Features")
	EChartFeatureType FeatureType = EChartFeatureType::AxisX;

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "NCharts|Features", meta = (ShowOnlyInnerProperties, NoClear))
	UNChartFeatureConfigBase* Config = nullptr;
};

UCLASS()
class NCHARTS_API ULineChartWidget : public UWidget
{
	GENERATED_BODY()

public:
	ULineChartWidget();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NCharts|Features")
	TArray<FNChartFeatureEntry> Features;

	UFUNCTION(BlueprintCallable, Category = "NCharts|LineChart")
	void SetPoints(const TArray<FVector2D>& InPoints);

	UFUNCTION(BlueprintCallable, Category = "NCharts|LineChart")
	void SetYLimit(float InLimit, bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "NCharts|LineChart")
	void SetLineColor(const FLinearColor& InColor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;

private:
	void SyncFeatureEntries();
	void ApplyFeatureEntriesToLiveProxies();

	TSharedPtr<FAxisXProxy> AxisXProxy;
	TSharedPtr<FAxisYProxy> AxisYProxy;
	TSharedPtr<FLineSeriesProxy> LineProxy;
	TSharedPtr<FTooltipProxy> TooltipProxy;
};
