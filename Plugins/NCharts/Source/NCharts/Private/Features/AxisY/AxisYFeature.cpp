// Copyright NCharts Plugin. All Rights Reserved.
// AxisY Feature 静态注册器

#include "Features/AxisY/AxisYFeatureConfig.h"
#include "Core/NChartRegistry.h"
#include "Features/AxisY/AxisYProxy.h"
#include "Features/AxisY/AxisYWidget.h"

namespace
{
	const FName AxisYFeatureName(TEXT("AxisY"));

	struct FAxisYFeatureRegistrar
	{
		FAxisYFeatureRegistrar()
		{
			FNChartRegistry::FNChartFeatureDescriptor Descriptor;
			Descriptor.Type = EChartFeatureType::AxisY;
			Descriptor.FeatureName = AxisYFeatureName;
			Descriptor.DisplayName = FText::FromString(TEXT("Axis Y"));
			Descriptor.ConfigClass = UAxisYFeatureConfig::StaticClass();
			Descriptor.ProxyFactory = []()
			{
				return StaticCastSharedRef<INChartProxy>(MakeShared<FAxisYProxy>());
			};
			Descriptor.WidgetFactory = [](const TSharedRef<INChartProxy>& InProxy)
			{
				const TSharedRef<FAxisYProxy> Proxy = StaticCastSharedRef<FAxisYProxy>(InProxy);
				return StaticCastSharedRef<SWidget>(SNew(SAxisYWidget).Proxy(Proxy));
			};
			FNChartRegistry::Get().RegisterFeature(Descriptor);
		}

		~FAxisYFeatureRegistrar()
		{
			FNChartRegistry::Get().UnregisterFeature(AxisYFeatureName);
		}
	};

	static FAxisYFeatureRegistrar AxisYRegistrar;
}
