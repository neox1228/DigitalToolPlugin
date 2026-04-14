#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "Core/NChartTypes.h"

class SWidget;
class UNChartFeatureConfigBase;

/**
 * 功能 Feature 注册器
 * 
 */
class NCHARTS_API FNChartRegistry
{
public:
	
	// 功能 Feature 函数类型重命名
	using FProxyFactory = TFunction<TSharedRef<INChartProxy>()>; // Proxy 
	using FWidgetFactory = TFunction<TSharedRef<SWidget>(const TSharedRef<INChartProxy>&)>; // Widget
	using FPostBuildLink = TFunction<void(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>&)>; // Feature依赖和绑定关系

	/**
	 * 功能 Feature 描述器
	 * @param Type：功能枚举类型
	 * @param FeatureName：功能名称
	 * @param DisplayName；展示名称
	 * @param ConfigClass：功能配置文件
	 * @param LayerOrder：绘制层级
	 * @param ProxyFactory：功能代理器
	 * @param WidgetFactory：绘制层
	 * @param 
	 */
	struct FNChartFeatureDescriptor
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

	// 单例模式
	static FNChartRegistry& Get();

	/**
	 * 功能 Feature 注册函数
	 * @param FeatureName 功能 Feature名称
	 * @param ProxyFactory 功能代理工厂
	 * @param WidgetFactory 功能绘制工厂
	 */
	void RegisterFeature(FName FeatureName, FProxyFactory ProxyFactory, FWidgetFactory WidgetFactory);

	/**
	 * 功能 Feature 注册函数（重载）
	 * @param Descriptor 功能 Feature 描述器
	 */
	void RegisterFeature(const FNChartFeatureDescriptor& Descriptor);

	/**
	 * 功能 Feature 反注册函数
	 * @param FeatureName 功能 Feature 名称
	 */
	void UnregisterFeature(FName FeatureName);

	/**
	 * 通过 Feature名称判断该 Feature 是否存在
	 * @param FeatureName 
	 * @return bool 
	 */
	bool HasFeature(FName FeatureName) const;

	/**
	 * 根据功能名称创建该功能的广播代理
	 * @param FeatureName 功能名称
	 * @return TSharedPtr<INChartProxy>
	 */
	TSharedPtr<INChartProxy> CreateProxy(FName FeatureName) const;

	/**
	 * 创建功能绘制对象
	 * @param FeatureName 功能 Feature 名称
	 * @param Proxy 功能代理
	 * @return TSharedPtr<SWidget>
	 */
	TSharedPtr<SWidget> CreateWidget(FName FeatureName, const TSharedRef<INChartProxy>& Proxy) const;

	/**
	 * 处理 Feature 之间的依赖和绑定关系
	 * @param FeatureProxies 当前功能 Feature 枚举和对应代理
	 */
	void ApplyPostBuildLinks(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies) const;

	/**
	 * 获取当前功能名称列表
	 * @return TArray<FName> 当前功能名称列表
	 */
	TArray<FName> GetFeatureNames() const;

	/**
	 * 获取当前功能类型列表
	 * @return TArray<EChartFeatureType> 当前功能类型列表
	 */
	TArray<EChartFeatureType> GetFeatureTypes() const;

	/**
	 * 通过 FeatureType 获取当前 Feature 的描述器
	 * @param Type 当前 Feature 类型
	 * @param OutDescriptor 当前 Feature 描述器
	 * @return bool
	 */
	bool GetFeatureDescriptorByType(EChartFeatureType Type, FNChartFeatureDescriptor& OutDescriptor) const;

	/**
	 * 
	 * @param FeatureName 当前 Feature 名称
	 * @param OutDescriptor 当前 Feature 描述器
	 * @return bool
	 */
	bool GetFeatureDescriptorByName(FName FeatureName, FNChartFeatureDescriptor& OutDescriptor) const;

private:
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

	TMap<FName, FFeatureEntry> Features;
	TMap<EChartFeatureType, FName> FeatureTypeToName;
};
