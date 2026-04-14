#include "Features/PointBubbles/PointBubblesFeatureConfig.h"

#include "Features/PointBubbles/PointBubblesProxy.h"

UPointBubblesFeatureConfig::UPointBubblesFeatureConfig()
{
	Points.Add(FVector2D(0.0f, 10.0f));
	Points.Add(FVector2D(1.0f, 18.0f));
	Points.Add(FVector2D(2.0f, 12.0f));
	Points.Add(FVector2D(3.0f, 26.0f));
	Points.Add(FVector2D(4.0f, 20.0f));
	Points.Add(FVector2D(5.0f, 32.0f));
	Points.Add(FVector2D(6.0f, 28.0f));
}

EChartFeatureType UPointBubblesFeatureConfig::GetFeatureType() const
{
	return EChartFeatureType::PointBubbles;
}

void UPointBubblesFeatureConfig::ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const
{
	const TSharedRef<FPointBubblesProxy> Proxy = StaticCastSharedRef<FPointBubblesProxy>(InProxy);
	Proxy->SetPoints(Points);
	Proxy->SetPadding(Padding);
	Proxy->SetPointRadius(PointRadius);
	Proxy->SetActivationDistance(ActivationDistance);
	Proxy->SetBreathAmplitude(BreathAmplitude);
	Proxy->SetBreathSpeed(BreathSpeed);
	Proxy->SetPointColor(PointColor);
	Proxy->SetHoverPointColor(HoverPointColor);
	Proxy->SetBubbleBackgroundColor(BubbleBackgroundColor);
	Proxy->SetBubbleTextColor(BubbleTextColor);
	Proxy->SetBubbleOffset(BubbleOffset);
}
