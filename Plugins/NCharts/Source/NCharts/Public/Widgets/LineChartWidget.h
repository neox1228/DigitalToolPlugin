#pragma once

#include "Core/NChartFeatureConfig.h"
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "LineChartWidget.generated.h"

class FAxisXProxy;
class FAxisYProxy;
class FLineSeriesProxy;
struct FPropertyChangedEvent;
struct FPropertyChangedChainEvent;

USTRUCT(BlueprintType)
struct FNChartFeatureEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NCharts|Features")
	EChartFeatureType FeatureType = EChartFeatureType::AxisX;

	/**
	 * 处理配置对象，实现在编辑器中直接内嵌显示、不可删除、且逻辑解耦的功能配置项
	 * 高级标签：
	 *    1. Instanced (独占对象)：当在包含这个Config的类被创建时，UE 会为每个实例对象自动克隆一份独立的 Config 对象
	 *    2. meta = (ShowOnlyInnerProperties) (去层级化)：加上后他会把该对象中的属性直接平铺在当前层级。
	 *    3. meta = (NoClear) (禁止置空)：在编辑器的下拉菜单中去掉 “Clear”选项
	 *    
	 */
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
	
	/**
	 * PostEditChangeProperty 当前对象的任何属性改变，就会触发；简单的属性监听
	 * PostEditChangeChainProperty 当嵌套对象内部的属性改变时触发；抓们处理嵌套属性的改变
	 */
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;

private:
	void SyncFeatureEntries();
	void ApplyFeatureEntriesToLiveProxies();

	TMap<EChartFeatureType, TSharedPtr<INChartProxy>> LiveFeatureProxies;
	TSharedPtr<FAxisXProxy> AxisXProxy;
	TSharedPtr<FAxisYProxy> AxisYProxy;
	TSharedPtr<FLineSeriesProxy> LineProxy;
};
