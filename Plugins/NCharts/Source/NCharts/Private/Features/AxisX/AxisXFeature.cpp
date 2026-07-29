// Copyright NCharts Plugin. All Rights Reserved.
// AxisX Feature 静态注册器：模块加载时自动向 FNChartRegistry 注册 X 轴功能

#include "Features/AxisX/AxisXFeatureConfig.h"
#include "Core/NChartRegistry.h"
#include "Features/AxisX/AxisXProxy.h"
#include "Features/AxisX/AxisXWidget.h"

namespace
{
	const FName AxisXFeatureName(TEXT("AxisX"));

	/**
	 * 静态注册器：构造时注册 Feature，析构时反注册
	 * 利用 C++ 静态对象初始化顺序，在模块加载阶段自动完成注册
	 */
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
