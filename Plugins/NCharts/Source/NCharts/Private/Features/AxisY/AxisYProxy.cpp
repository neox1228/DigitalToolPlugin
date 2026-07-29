// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/AxisY/AxisYProxy.h"

const FAxisYState& FAxisYProxy::GetState() const
{
	return State;
}

FAxisYProxy::FOnStateChanged& FAxisYProxy::OnStateChanged()
{
	return StateChanged;
}

void FAxisYProxy::SetAxisColor(const FLinearColor& InColor)
{
	State.AxisColor = InColor;
	StateChanged.Broadcast();
}

void FAxisYProxy::SetAxisThickness(float InThickness)
{
	State.AxisThickness = InThickness;
	StateChanged.Broadcast();
}

void FAxisYProxy::SetPadding(const FVector2D& InPadding)
{
	State.Padding = InPadding;
	StateChanged.Broadcast();
}

void FAxisYProxy::SetShowTicks(bool bEnable)
{
	State.bShowTicks = bEnable;
	StateChanged.Broadcast();
}

void FAxisYProxy::SetShowLabels(bool bEnable)
{
	State.bShowLabels = bEnable;
	StateChanged.Broadcast();
}

void FAxisYProxy::SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale)
{
	CartesianScale = InScale;
	StateChanged.Broadcast();
}

TSharedPtr<FNChartCartesianScale> FAxisYProxy::GetCartesianScale() const
{
	return CartesianScale;
}
