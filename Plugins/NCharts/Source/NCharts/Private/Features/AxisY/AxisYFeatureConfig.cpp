// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/AxisY/AxisYFeatureConfig.h"

#include "Features/AxisY/AxisYProxy.h"

EChartFeatureType UAxisYFeatureConfig::GetFeatureType() const
{
	return EChartFeatureType::AxisY;
}

void UAxisYFeatureConfig::ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const
{
	const TSharedRef<FAxisYProxy> AxisProxy = StaticCastSharedRef<FAxisYProxy>(InProxy);
	AxisProxy->SetAxisColor(AxisColor);
	AxisProxy->SetAxisThickness(AxisThickness);
	AxisProxy->SetPadding(Padding);
	AxisProxy->SetShowTicks(bShowTicks);
	AxisProxy->SetShowLabels(bShowLabels);
}
