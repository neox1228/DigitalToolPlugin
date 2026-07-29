// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "Core/NChartTypes.h"

class SWidget;
class UNChartFeatureConfigBase;

/**
 * FNChartRegistry —— 图表 Feature 注册中心（单例）
 *
 * 核心职责：
 * 1. 管理所有可插拔 Feature 的元数据（类型、配置类、工厂函数、绘制层级）
 * 2. 提供 Proxy / Widget 的工厂方法，供 ULineChartWidget 在 RebuildWidget 时调用
 * 3. 在全部 Feature 创建完成后，执行 PostBuildLink 建立跨 Feature 依赖（如 Tooltip 绑定 LineSeries）
 *
 * 注册方式：各 Feature 的 *Feature.cpp 中使用静态 Registrar 在模块加载时自动注册，
 * 无需在 FNChartsModule::StartupModule 中手动调用。
 */
class NCHARTS_API FNChartRegistry
{
public:
	/** Proxy 工厂：无参，返回对应 Feature 的 INChartProxy 共享实例 */
	using FProxyFactory = TFunction<TSharedRef<INChartProxy>()>;

	/** Widget 工厂：接收 Proxy 引用，返回对应的 Slate 绘制控件 */
	using FWidgetFactory = TFunction<TSharedRef<SWidget>(const TSharedRef<INChartProxy>&)>;

	/**
	 * PostBuildLink 回调：所有 Feature Proxy 创建完毕后调用
	 * 用于建立 Feature 间的运行时引用关系（如 Tooltip → LineSeries）
	 */
	using FPostBuildLink = TFunction<void(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>&)>;

	/**
	 * Feature 描述符 —— 注册时提交的完整元数据包
	 * 包含类型标识、配置类、工厂函数、绘制层级及依赖绑定回调
	 */
	struct FNChartFeatureDescriptor
	{
		EChartFeatureType Type = EChartFeatureType::AxisX;
		FName FeatureName = NAME_None;
		FText DisplayName;
		TSubclassOf<UNChartFeatureConfigBase> ConfigClass;
		int32 LayerOrder = 0;  // 绘制层级，数值越大越靠上（Tooltip 默认为 100）
		FProxyFactory ProxyFactory;
		FWidgetFactory WidgetFactory;
		FPostBuildLink PostBuildLink;
	};

	/** 获取全局唯一 Registry 实例 */
	static FNChartRegistry& Get();

	/** 简化版注册：仅提供名称和两个工厂函数 */
	void RegisterFeature(FName FeatureName, FProxyFactory ProxyFactory, FWidgetFactory WidgetFactory);

	/** 完整版注册：提交包含类型、配置类、层级等全部信息的描述符 */
	void RegisterFeature(const FNChartFeatureDescriptor& Descriptor);

	/** 反注册：模块卸载或静态 Registrar 析构时调用 */
	void UnregisterFeature(FName FeatureName);

	/** 判断指定名称的 Feature 是否已注册 */
	bool HasFeature(FName FeatureName) const;

	/** 根据 Feature 名称创建 Proxy 实例 */
	TSharedPtr<INChartProxy> CreateProxy(FName FeatureName) const;

	/** 根据 Feature 名称和已有 Proxy 创建对应的 Slate Widget */
	TSharedPtr<SWidget> CreateWidget(FName FeatureName, const TSharedRef<INChartProxy>& Proxy) const;

	/**
	 * 执行所有已注册 Feature 的 PostBuildLink 回调
	 * @param FeatureProxies 当前图表实例中已创建的全部 Proxy，按类型索引
	 */
	void ApplyPostBuildLinks(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies) const;

	/** 获取所有已注册 Feature 的名称列表 */
	TArray<FName> GetFeatureNames() const;

	/** 获取所有已注册 Feature 的类型列表 */
	TArray<EChartFeatureType> GetFeatureTypes() const;

	/** 通过 Feature 类型查找描述符 */
	bool GetFeatureDescriptorByType(EChartFeatureType Type, FNChartFeatureDescriptor& OutDescriptor) const;

	/** 通过 Feature 名称查找描述符 */
	bool GetFeatureDescriptorByName(FName FeatureName, FNChartFeatureDescriptor& OutDescriptor) const;

private:
	/** 内部存储条目，与 FNChartFeatureDescriptor 字段一一对应 */
	struct FFeatureEntry
	{
		EChartFeatureType Type = EChartFeatureType::AxisX;
		FName FeatureName = NAME_None;
		FText DisplayName;
		TSubclassOf<UNChartFeatureConfigBase> ConfigClass;
		int32 LayerOrder = 0;
		FProxyFactory ProxyFactory;
		FWidgetFactory WidgetFactory;
		FPostBuildLink PostBuildLink;
	};

	/** Feature 名称 → 条目映射 */
	TMap<FName, FFeatureEntry> Features;

	/** Feature 类型 → 名称映射（每种类型只允许注册一个 Feature） */
	TMap<EChartFeatureType, FName> FeatureTypeToName;
};
