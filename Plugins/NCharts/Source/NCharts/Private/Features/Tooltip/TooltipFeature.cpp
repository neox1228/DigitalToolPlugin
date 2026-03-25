#include "Features/Tooltip/TooltipFeatureConfig.h"
#include "Core/NChartRegistry.h"
#include "Features/Tooltip/TooltipProxy.h"
#include "Features/Tooltip/TooltipWidget.h"

namespace
{
	const FName TooltipFeatureName(TEXT("Tooltip"));

	struct FTooltipFeatureRegistrar
	{
		FTooltipFeatureRegistrar()
		{
			FNChartRegistry::FNChartFeatureDescriptor Descriptor;
			Descriptor.Type = EChartFeatureType::Tooltip;
			Descriptor.FeatureName = TooltipFeatureName;
			Descriptor.DisplayName = FText::FromString(TEXT("Tooltip"));
			Descriptor.ConfigClass = UTooltipFeatureConfig::StaticClass();
			Descriptor.ProxyFactory = []()
			{
				return StaticCastSharedRef<INChartProxy>(MakeShared<FTooltipProxy>());
			};
			Descriptor.WidgetFactory = [](const TSharedRef<INChartProxy>& InProxy)
			{
				const TSharedRef<FTooltipProxy> Proxy = StaticCastSharedRef<FTooltipProxy>(InProxy);
				return StaticCastSharedRef<SWidget>(SNew(STooltipWidget).Proxy(Proxy));
			};
			FNChartRegistry::Get().RegisterFeature(Descriptor);
		}

		~FTooltipFeatureRegistrar()
		{
			FNChartRegistry::Get().UnregisterFeature(TooltipFeatureName);
		}
	};

	static FTooltipFeatureRegistrar TooltipRegistrar;
}
