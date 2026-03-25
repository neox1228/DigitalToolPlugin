#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "Core/NChartTypes.h"

class SWidget;
class UNChartFeatureConfigBase;

class NCHARTS_API FNChartRegistry
{
public:
	using FProxyFactory = TFunction<TSharedRef<INChartProxy>()>;
	using FWidgetFactory = TFunction<TSharedRef<SWidget>(const TSharedRef<INChartProxy>&)>;

	struct FNChartFeatureDescriptor
	{
		EChartFeatureType Type = EChartFeatureType::AxisX;
		FName FeatureName = NAME_None;
		FText DisplayName;
		TSubclassOf<UNChartFeatureConfigBase> ConfigClass;
		FProxyFactory ProxyFactory;
		FWidgetFactory WidgetFactory;
	};

	static FNChartRegistry& Get();

	void RegisterFeature(FName FeatureName, FProxyFactory ProxyFactory, FWidgetFactory WidgetFactory);
	void RegisterFeature(const FNChartFeatureDescriptor& Descriptor);
	void UnregisterFeature(FName FeatureName);

	bool HasFeature(FName FeatureName) const;

	TSharedPtr<INChartProxy> CreateProxy(FName FeatureName) const;
	TSharedPtr<SWidget> CreateWidget(FName FeatureName, const TSharedRef<INChartProxy>& Proxy) const;

	TArray<FName> GetFeatureNames() const;
	TArray<EChartFeatureType> GetFeatureTypes() const;

	bool GetFeatureDescriptorByType(EChartFeatureType Type, FNChartFeatureDescriptor& OutDescriptor) const;
	bool GetFeatureDescriptorByName(FName FeatureName, FNChartFeatureDescriptor& OutDescriptor) const;

private:
	struct FFeatureEntry
	{
		EChartFeatureType Type = EChartFeatureType::AxisX;
		FName FeatureName = NAME_None;
		FText DisplayName;
		TSubclassOf<UNChartFeatureConfigBase> ConfigClass;
		FProxyFactory ProxyFactory;
		FWidgetFactory WidgetFactory;
	};

	TMap<FName, FFeatureEntry> Features;
	TMap<EChartFeatureType, FName> FeatureTypeToName;
};
