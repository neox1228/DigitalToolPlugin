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
