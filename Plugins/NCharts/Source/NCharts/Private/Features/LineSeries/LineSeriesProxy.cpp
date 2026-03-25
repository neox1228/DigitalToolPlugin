#include "Features/LineSeries/LineSeriesProxy.h"

TSharedRef<FLineSeriesProxy> FLineSeriesProxy::CreateDemo()
{
	TSharedRef<FLineSeriesProxy> Proxy = MakeShared<FLineSeriesProxy>();

	TArray<FVector2D> DemoPoints;
	DemoPoints.Add(FVector2D(0.0, 10.0));
	DemoPoints.Add(FVector2D(1.0, 18.0));
	DemoPoints.Add(FVector2D(2.0, 12.0));
	DemoPoints.Add(FVector2D(3.0, 26.0));
	DemoPoints.Add(FVector2D(4.0, 20.0));
	DemoPoints.Add(FVector2D(5.0, 32.0));
	DemoPoints.Add(FVector2D(6.0, 28.0));

	Proxy->SetPoints(DemoPoints);
	Proxy->SetYLimit(25.0f, true);
	return Proxy;
}

const FLineSeriesState& FLineSeriesProxy::GetState() const
{
	return State;
}

FLineSeriesProxy::FOnStateChanged& FLineSeriesProxy::OnStateChanged()
{
	return StateChanged;
}

void FLineSeriesProxy::SetPoints(const TArray<FVector2D>& InPoints)
{
	State.Points = InPoints;
	UpdateDerivedState();
	StateChanged.Broadcast();
}

void FLineSeriesProxy::SetLineColor(const FLinearColor& InColor)
{
	State.BaseLineColor = InColor;
	UpdateDerivedState();
	StateChanged.Broadcast();
}

void FLineSeriesProxy::SetYLimit(float InLimit, bool bEnable)
{
	State.YLimit = InLimit;
	State.bUseLimit = bEnable;
	UpdateDerivedState();
	StateChanged.Broadcast();
}

void FLineSeriesProxy::SetPadding(const FVector2D& InPadding)
{
	State.Padding = InPadding;
	StateChanged.Broadcast();
}

void FLineSeriesProxy::UpdateDerivedState()
{
	State.bOverLimit = false;
	if (State.bUseLimit)
	{
		for (const FVector2D& Point : State.Points)
		{
			if (Point.Y > State.YLimit)
			{
				State.bOverLimit = true;
				break;
			}
		}
	}

	State.ActiveLineColor = State.bOverLimit ? State.OverLimitColor : State.BaseLineColor;
}
