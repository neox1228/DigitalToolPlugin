// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "Core/NChartFeatureConfig.h"
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "UObject/ObjectSaveContext.h"
#include "BaseChartWidget.generated.h"

struct FPropertyChangedEvent;
struct FPropertyChangedChainEvent;
class FNChartCartesianScale;

/**
 * FNChartFeatureEntry —— 单个 Feature 在 Chart Widget 中的配置条目
 *
 * 等价于 ECharts option 中的一个配置块（如 xAxis、series[0]、tooltip）。
 * 在编辑器 Details 面板增删此数组条目，即组合不同的图表能力。
 */
USTRUCT(BlueprintType)
struct FNChartFeatureEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NCharts|Features")
	EChartFeatureType FeatureType = EChartFeatureType::AxisX;

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "NCharts|Features", meta = (ShowOnlyInnerProperties, NoClear))
	UNChartFeatureConfigBase* Config = nullptr;
};

/**
 * UBaseChartWidget —— 图表 UMG 基类
 *
 * Features 默认为空，由子类或用户在编辑器中自由增删 Feature 条目来组装图表。
 * 子类（如 ULineChartWidget）可在构造函数中注入预设 Feature 列表作为模板。
 */
UCLASS(Abstract, Blueprintable)
class NCHARTS_API UBaseChartWidget : public UWidget
{
	GENERATED_BODY()

public:
	UBaseChartWidget();

	/** 图表功能模块列表，默认为空，等价于 ECharts 的 option 根对象 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NCharts|Features")
	TArray<FNChartFeatureEntry> Features;

	/** 向 Features 末尾添加指定类型的 Feature 条目（若已存在同类型则跳过） */
	UFUNCTION(BlueprintCallable, Category = "NCharts|Chart")
	bool AddFeature(EChartFeatureType FeatureType);

	/** 按索引移除 Feature 条目 */
	UFUNCTION(BlueprintCallable, Category = "NCharts|Chart")
	bool RemoveFeatureAt(int32 Index);

	/** 是否已包含指定类型的 Feature */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NCharts|Chart")
	bool HasFeature(EChartFeatureType FeatureType) const;

	/** 获取指定类型 Feature 的 Config 对象（蓝图可 Cast 为具体 Config 类型） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NCharts|Chart")
	UNChartFeatureConfigBase* GetFeatureConfig(EChartFeatureType FeatureType) const;

	/**
	 * 为支持点数据的 Feature 设置数据（LineSeries / PointBubbles）
	 * 同时写入 Config 与 Live Proxy，保证序列化与即时渲染一致
	 */
	UFUNCTION(BlueprintCallable, Category = "NCharts|Chart")
	void SetFeaturePoints(EChartFeatureType FeatureType, const TArray<FVector2D>& InPoints);

	/** 将当前全部 Feature Config 重新应用到 Live Proxy（不重建 Slate 树） */
	UFUNCTION(BlueprintCallable, Category = "NCharts|Chart")
	void RefreshFeatureProxies();

	/** 强制重建 Slate 控件树（增删 Feature 后若未自动刷新可手动调用） */
	UFUNCTION(BlueprintCallable, Category = "NCharts|Chart")
	void RebuildChart();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;

	/** 子类可重写：在 CDO 或实例创建时注入默认 Feature 预设 */
	virtual void InitializePresetFeatures();

	/**
	 * 确保 Features[].Config 类型/Outer/Archetype 合法。
	 * CDO 上不创建 Config；实例上若 Archetype 指向 CDO 私有子对象则重建，
	 * 否则 Widget 蓝图保存会报 Illegal reference to private object。
	 */
	void SyncFeatureEntries();
	void ApplyFeatureEntriesToLiveProxies();
	void ResetLiveProxies();
	void BuildAndApplyCartesianScale();

	TSharedPtr<INChartProxy> GetLiveFeatureProxy(EChartFeatureType FeatureType) const;

	TMap<EChartFeatureType, TSharedPtr<INChartProxy>> LiveFeatureProxies;
	TSharedPtr<FNChartCartesianScale> LiveCartesianScale;
};
