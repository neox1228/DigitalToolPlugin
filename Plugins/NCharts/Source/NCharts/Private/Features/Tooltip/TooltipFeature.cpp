// Copyright NCharts Plugin. All Rights Reserved.

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
			Descriptor.LayerOrder = 100;
			Descriptor.ProxyFactory = []()
			{
				return StaticCastSharedRef<INChartProxy>(MakeShared<FTooltipProxy>());
			};
			Descriptor.WidgetFactory = [](const TSharedRef<INChartProxy>& InProxy)
			{
				const TSharedRef<FTooltipProxy> Proxy = StaticCastSharedRef<FTooltipProxy>(InProxy);
				return StaticCastSharedRef<SWidget>(SNew(STooltipWidget).Proxy(Proxy));
			};
			Descriptor.PostBuildLink = [](const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies)
			{
				const TSharedPtr<INChartProxy>* TooltipBase = FeatureProxies.Find(EChartFeatureType::Tooltip);
				if (!TooltipBase || !TooltipBase->IsValid())
				{
					return;
				}

				const TSharedPtr<FTooltipProxy> TooltipProxy = StaticCastSharedPtr<FTooltipProxy>(*TooltipBase);
				if (TooltipProxy.IsValid())
				{
					TooltipProxy->SetDataProviders(FeatureProxies);
				}
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
