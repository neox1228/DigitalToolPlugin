#include "Features/Tooltip/TooltipFeatureConfig.h"

#include "Features/Tooltip/TooltipProxy.h"

EChartFeatureType UTooltipFeatureConfig::GetFeatureType() const
{
	return EChartFeatureType::Tooltip;
}

void UTooltipFeatureConfig::ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const
{
	const TSharedRef<FTooltipProxy> TooltipProxy = StaticCastSharedRef<FTooltipProxy>(InProxy);
	TooltipProxy->SetTooltipEnabled(bEnableTooltip);
	TooltipProxy->SetShowHoverLine(bShowHoverLine);
	TooltipProxy->SetShowHoverPoint(bShowHoverPoint);
	TooltipProxy->SetActivationDistance(ActivationDistance);
	TooltipProxy->SetHoverPointRadius(HoverPointRadius);
	TooltipProxy->SetHoverLineColor(HoverLineColor);
	TooltipProxy->SetHoverPointColor(HoverPointColor);
	TooltipProxy->SetTooltipBackgroundColor(TooltipBackgroundColor);
	TooltipProxy->SetTooltipTextColor(TooltipTextColor);
	TooltipProxy->SetTooltipOffset(TooltipOffset);
}
