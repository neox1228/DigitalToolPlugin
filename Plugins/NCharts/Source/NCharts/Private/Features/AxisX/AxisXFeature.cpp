#include "Features/AxisX/AxisXFeatureConfig.h"
#include "Core/NChartRegistry.h"
#include "Features/AxisX/AxisXProxy.h"
#include "Features/AxisX/AxisXWidget.h"

namespace
{
	const FName AxisXFeatureName(TEXT("AxisX"));

	struct FAxisXFeatureRegistrar
	{
		FAxisXFeatureRegistrar()
		{
			FNChartRegistry::FNChartFeatureDescriptor Descriptor;
			Descriptor.Type = EChartFeatureType::AxisX;
			Descriptor.FeatureName = AxisXFeatureName;
			Descriptor.DisplayName = FText::FromString(TEXT("Axis X"));
			Descriptor.ConfigClass = UAxisXFeatureConfig::StaticClass();
			Descriptor.ProxyFactory = []()
			{
				return StaticCastSharedRef<INChartProxy>(MakeShared<FAxisXProxy>());
			};
			Descriptor.WidgetFactory = [](const TSharedRef<INChartProxy>& InProxy)
			{
				const TSharedRef<FAxisXProxy> Proxy = StaticCastSharedRef<FAxisXProxy>(InProxy);
				return StaticCastSharedRef<SWidget>(SNew(SAxisXWidget).Proxy(Proxy));
			};
			FNChartRegistry::Get().RegisterFeature(Descriptor);
		}

		~FAxisXFeatureRegistrar()
		{
			FNChartRegistry::Get().UnregisterFeature(AxisXFeatureName);
		}
	};

	static FAxisXFeatureRegistrar AxisXRegistrar;
}
