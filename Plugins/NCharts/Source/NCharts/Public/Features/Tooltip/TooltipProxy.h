// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartCartesianScale.h"
#include "Core/NChartTypes.h"
#include "Features/Tooltip/TooltipState.h"

class NCHARTS_API FTooltipProxy : public TSharedFromThis<FTooltipProxy>, public INChartProxy, public INChartScaleConsumer
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	const FTooltipState& GetState() const;
	FOnStateChanged& OnStateChanged();

	/** PostBuildLink：从 Chart 全部 Proxy 中自动收集数据提供者 */
	void SetDataProviders(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies);

	const TArray<TWeakPtr<INChartProxy>>& GetDataProviders() const { return DataProviders; }

	void SetTrigger(EChartTooltipTrigger InTrigger);
	void SetTooltipEnabled(bool bEnable);
	void SetShowHoverLine(bool bEnable);
	void SetShowHoverPoint(bool bEnable);
	void SetActivationDistance(float InDistance);
	void SetHoverLineColor(const FLinearColor& InColor);
	void SetHoverPointColor(const FLinearColor& InColor);
	void SetTooltipBackgroundColor(const FLinearColor& InColor);
	void SetTooltipTextColor(const FLinearColor& InColor);
	void SetHoverPointRadius(float InRadius);
	void SetTooltipOffset(const FVector2D& InOffset);

	void SetActiveTooltip(const TArray<FTooltipParam>& InParams, float InAxisScreenX, const FVector2D& InDrawMin, const FVector2D& InDrawMax);
	void ClearHoverState();

	virtual INChartScaleConsumer* GetScaleConsumer() override { return this; }
	virtual void SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale) override;
	virtual TSharedPtr<FNChartCartesianScale> GetCartesianScale() const override;

private:
	FTooltipState State;
	TArray<TWeakPtr<INChartProxy>> DataProviders;
	TSharedPtr<FNChartCartesianScale> CartesianScale;
	FOnStateChanged StateChanged;
};
