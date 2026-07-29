// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/Tooltip/TooltipProxy.h"

#include "Core/NChartCartesianScale.h"

const FTooltipState& FTooltipProxy::GetState() const
{
	return State;
}

FTooltipProxy::FOnStateChanged& FTooltipProxy::OnStateChanged()
{
	return StateChanged;
}

void FTooltipProxy::SetDataProviders(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies)
{
	DataProviders.Reset();

	for (const TPair<EChartFeatureType, TSharedPtr<INChartProxy>>& Pair : FeatureProxies)
	{
		if (Pair.Key == EChartFeatureType::Tooltip)
		{
			continue;
		}

		const TSharedPtr<INChartProxy>& Proxy = Pair.Value;
		if (Proxy.IsValid() && Proxy->GetTooltipDataProvider())
		{
			DataProviders.Add(Proxy);
		}
	}

	StateChanged.Broadcast();
}

void FTooltipProxy::SetTrigger(EChartTooltipTrigger InTrigger)
{
	State.Trigger = InTrigger;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetTooltipEnabled(bool bEnable)
{
	State.bEnableTooltip = bEnable;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetShowHoverLine(bool bEnable)
{
	State.bShowHoverLine = bEnable;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetShowHoverPoint(bool bEnable)
{
	State.bShowHoverPoint = bEnable;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetActivationDistance(float InDistance)
{
	State.ActivationDistance = InDistance;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetHoverLineColor(const FLinearColor& InColor)
{
	State.HoverLineColor = InColor;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetHoverPointColor(const FLinearColor& InColor)
{
	State.HoverPointColor = InColor;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetTooltipBackgroundColor(const FLinearColor& InColor)
{
	State.TooltipBackgroundColor = InColor;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetTooltipTextColor(const FLinearColor& InColor)
{
	State.TooltipTextColor = InColor;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetHoverPointRadius(float InRadius)
{
	State.HoverPointRadius = InRadius;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetTooltipOffset(const FVector2D& InOffset)
{
	State.TooltipOffset = InOffset;
	StateChanged.Broadcast();
}

void FTooltipProxy::SetActiveTooltip(
	const TArray<FTooltipParam>& InParams,
	float InAxisScreenX,
	const FVector2D& InDrawMin,
	const FVector2D& InDrawMax)
{
	State.bHasHover = InParams.Num() > 0;
	State.ActiveParams = InParams;
	State.AxisScreenX = InAxisScreenX;
	State.DrawMin = InDrawMin;
	State.DrawMax = InDrawMax;
	StateChanged.Broadcast();
}

void FTooltipProxy::ClearHoverState()
{
	if (State.bHasHover)
	{
		State.bHasHover = false;
		State.ActiveParams.Reset();
		State.AxisScreenX = 0.0f;
		StateChanged.Broadcast();
	}
}

void FTooltipProxy::SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale)
{
	CartesianScale = InScale;
	StateChanged.Broadcast();
}

TSharedPtr<FNChartCartesianScale> FTooltipProxy::GetCartesianScale() const
{
	return CartesianScale;
}
