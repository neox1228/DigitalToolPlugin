// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/AxisX/AxisXProxy.h"

const FAxisXState& FAxisXProxy::GetState() const
{
	return State;
}

FAxisXProxy::FOnStateChanged& FAxisXProxy::OnStateChanged()
{
	return StateChanged;
}

void FAxisXProxy::SetAxisColor(const FLinearColor& InColor)
{
	State.AxisColor = InColor;
	StateChanged.Broadcast();
}

void FAxisXProxy::SetAxisThickness(float InThickness)
{
	State.AxisThickness = InThickness;
	StateChanged.Broadcast();
}

void FAxisXProxy::SetPadding(const FVector2D& InPadding)
{
	State.Padding = InPadding;
	StateChanged.Broadcast();
}

void FAxisXProxy::SetShowTicks(bool bEnable)
{
	State.bShowTicks = bEnable;
	StateChanged.Broadcast();
}

void FAxisXProxy::SetShowLabels(bool bEnable)
{
	State.bShowLabels = bEnable;
	StateChanged.Broadcast();
}

void FAxisXProxy::SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale)
{
	CartesianScale = InScale;
	StateChanged.Broadcast();
}

TSharedPtr<FNChartCartesianScale> FAxisXProxy::GetCartesianScale() const
{
	return CartesianScale;
}
