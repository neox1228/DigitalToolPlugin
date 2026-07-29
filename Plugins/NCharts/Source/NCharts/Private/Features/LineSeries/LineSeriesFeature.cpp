// Copyright NCharts Plugin. All Rights Reserved.
// LineSeries Feature 静态注册器

#include "Features/LineSeries/LineSeriesFeatureConfig.h"
#include "Core/NChartRegistry.h"
#include "Features/LineSeries/LineSeriesProxy.h"
#include "Features/LineSeries/LineSeriesWidget.h"

namespace
{
	const FName LineSeriesFeatureName(TEXT("LineSeries"));

	struct FLineSeriesFeatureRegistrar
	{
		FLineSeriesFeatureRegistrar()
		{
			FNChartRegistry::FNChartFeatureDescriptor Descriptor;
			Descriptor.Type = EChartFeatureType::LineSeries;
			Descriptor.FeatureName = LineSeriesFeatureName;
			Descriptor.DisplayName = FText::FromString(TEXT("Line Series"));
			Descriptor.ConfigClass = ULineSeriesFeatureConfig::StaticClass();
			Descriptor.ProxyFactory = []()
			{
				return StaticCastSharedRef<INChartProxy>(FLineSeriesProxy::CreateDemo());
			};
			Descriptor.WidgetFactory = [](const TSharedRef<INChartProxy>& InProxy)
			{
				const TSharedRef<FLineSeriesProxy> Proxy = StaticCastSharedRef<FLineSeriesProxy>(InProxy);
				return StaticCastSharedRef<SWidget>(SNew(SLineSeriesWidget).Proxy(Proxy));
			};
			FNChartRegistry::Get().RegisterFeature(Descriptor);
		}

		~FLineSeriesFeatureRegistrar()
		{
			FNChartRegistry::Get().UnregisterFeature(LineSeriesFeatureName);
		}
	};

	static FLineSeriesFeatureRegistrar LineSeriesRegistrar;
}
