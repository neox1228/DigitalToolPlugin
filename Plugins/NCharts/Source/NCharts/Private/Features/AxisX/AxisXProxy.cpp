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
