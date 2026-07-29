// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/AxisX/AxisXFeatureConfig.h"

#include "Features/AxisX/AxisXProxy.h"

EChartFeatureType UAxisXFeatureConfig::GetFeatureType() const
{
	return EChartFeatureType::AxisX;
}

void UAxisXFeatureConfig::ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const
{
	const TSharedRef<FAxisXProxy> AxisProxy = StaticCastSharedRef<FAxisXProxy>(InProxy);
	AxisProxy->SetAxisColor(AxisColor);
	AxisProxy->SetAxisThickness(AxisThickness);
	AxisProxy->SetPadding(Padding);
	AxisProxy->SetShowTicks(bShowTicks);
	AxisProxy->SetShowLabels(bShowLabels);
}
