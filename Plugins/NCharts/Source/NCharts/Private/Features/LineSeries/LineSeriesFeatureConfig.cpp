#include "Features/LineSeries/LineSeriesFeatureConfig.h"

#include "Features/LineSeries/LineSeriesProxy.h"

ULineSeriesFeatureConfig::ULineSeriesFeatureConfig()
{
	Points.Add(FVector2D(0.0, 10.0));
	Points.Add(FVector2D(1.0, 18.0));
	Points.Add(FVector2D(2.0, 12.0));
	Points.Add(FVector2D(3.0, 26.0));
	Points.Add(FVector2D(4.0, 20.0));
	Points.Add(FVector2D(5.0, 32.0));
	Points.Add(FVector2D(6.0, 28.0));
}

EChartFeatureType ULineSeriesFeatureConfig::GetFeatureType() const
{
	return EChartFeatureType::LineSeries;
}

void ULineSeriesFeatureConfig::ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const
{
	const TSharedRef<FLineSeriesProxy> LineProxy = StaticCastSharedRef<FLineSeriesProxy>(InProxy);
	LineProxy->SetPoints(Points);
	LineProxy->SetLineColor(LineColor);
	LineProxy->SetPadding(Padding);
	LineProxy->SetYLimit(YLimit, bUseLimit);
}
