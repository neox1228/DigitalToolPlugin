#include "Features/PointBubbles/PointBubblesFeatureConfig.h"

#include "Core/NChartRegistry.h"
#include "Features/PointBubbles/PointBubblesProxy.h"
#include "Features/PointBubbles/PointBubblesWidget.h"

namespace
{
	const FName PointBubblesFeatureName(TEXT("PointBubbles"));

	struct FPointBubblesFeatureRegistrar
	{
		FPointBubblesFeatureRegistrar()
		{
			FNChartRegistry::FNChartFeatureDescriptor Descriptor;
			Descriptor.Type = EChartFeatureType::PointBubbles;
			Descriptor.FeatureName = PointBubblesFeatureName;
			Descriptor.DisplayName = FText::FromString(TEXT("Point Bubbles"));
			Descriptor.ConfigClass = UPointBubblesFeatureConfig::StaticClass();
			Descriptor.ProxyFactory = []()
			{
				return StaticCastSharedRef<INChartProxy>(FPointBubblesProxy::CreateDemo());
			};
			Descriptor.WidgetFactory = [](const TSharedRef<INChartProxy>& InProxy)
			{
				const TSharedRef<FPointBubblesProxy> Proxy = StaticCastSharedRef<FPointBubblesProxy>(InProxy);
				return StaticCastSharedRef<SWidget>(SNew(SPointBubblesWidget).Proxy(Proxy));
			};
			FNChartRegistry::Get().RegisterFeature(Descriptor);
		}

		~FPointBubblesFeatureRegistrar()
		{
			FNChartRegistry::Get().UnregisterFeature(PointBubblesFeatureName);
		}
	};

	static FPointBubblesFeatureRegistrar PointBubblesRegistrar;
}
