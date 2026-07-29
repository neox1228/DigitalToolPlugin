// Copyright NCharts Plugin. All Rights Reserved.

#include "Widgets/LineChartWidget.h"

#include "Features/LineSeries/LineSeriesFeatureConfig.h"
#include "Features/LineSeries/LineSeriesProxy.h"

ULineChartWidget::ULineChartWidget()
{
}

void ULineChartWidget::InitializePresetFeatures()
{
	// 仅在 Features 为空时注入预设，避免覆盖用户在编辑器中的自定义配置
	if (!Features.IsEmpty())
	{
		return;
	}

	AddFeature(EChartFeatureType::AxisX);
	AddFeature(EChartFeatureType::AxisY);
	AddFeature(EChartFeatureType::LineSeries);
	AddFeature(EChartFeatureType::Tooltip);
}

void ULineChartWidget::SetPoints(const TArray<FVector2D>& InPoints)
{
	SetFeaturePoints(EChartFeatureType::LineSeries, InPoints);
}

void ULineChartWidget::SetYLimit(float InLimit, bool bEnable)
{
	if (ULineSeriesFeatureConfig* Config = Cast<ULineSeriesFeatureConfig>(GetFeatureConfig(EChartFeatureType::LineSeries)))
	{
		Config->YLimit = InLimit;
		Config->bUseLimit = bEnable;
	}

	if (TSharedPtr<FLineSeriesProxy> Proxy = StaticCastSharedPtr<FLineSeriesProxy>(GetLiveFeatureProxy(EChartFeatureType::LineSeries)))
	{
		Proxy->SetYLimit(InLimit, bEnable);
	}
}

void ULineChartWidget::SetLineColor(const FLinearColor& InColor)
{
	if (ULineSeriesFeatureConfig* Config = Cast<ULineSeriesFeatureConfig>(GetFeatureConfig(EChartFeatureType::LineSeries)))
	{
		Config->LineColor = InColor;
	}

	if (TSharedPtr<FLineSeriesProxy> Proxy = StaticCastSharedPtr<FLineSeriesProxy>(GetLiveFeatureProxy(EChartFeatureType::LineSeries)))
	{
		Proxy->SetLineColor(InColor);
	}
}
