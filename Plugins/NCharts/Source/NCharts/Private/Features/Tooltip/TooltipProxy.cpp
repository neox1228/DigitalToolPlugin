#include "Features/Tooltip/TooltipProxy.h"

#include "Features/LineSeries/LineSeriesProxy.h"

const FTooltipState& FTooltipProxy::GetState() const
{
	return State;
}

FTooltipProxy::FOnStateChanged& FTooltipProxy::OnStateChanged()
{
	return StateChanged;
}

void FTooltipProxy::SetTargetLineProxy(const TSharedPtr<FLineSeriesProxy>& InTargetLineProxy)
{
	TargetLineProxy = InTargetLineProxy;
	StateChanged.Broadcast();
}

TSharedPtr<FLineSeriesProxy> FTooltipProxy::GetTargetLineProxy() const
{
	return TargetLineProxy.Pin();
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

void FTooltipProxy::SetHoverState(int32 InIndex, const FVector2D& InDataPoint, const FVector2D& InScreenPoint)
{
	State.bHasHover = true;
	State.HoveredIndex = InIndex;
	State.HoveredDataPoint = InDataPoint;
	State.HoveredScreenPoint = InScreenPoint;
	StateChanged.Broadcast();
}

void FTooltipProxy::ClearHoverState()
{
	if (State.bHasHover)
	{
		State.bHasHover = false;
		State.HoveredIndex = INDEX_NONE;
		State.HoveredDataPoint = FVector2D::ZeroVector;
		State.HoveredScreenPoint = FVector2D::ZeroVector;
		StateChanged.Broadcast();
	}
}
