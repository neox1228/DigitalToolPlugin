#include "Features/PointBubbles/PointBubblesProxy.h"

#include "Core/NChartMathLib.h"

TSharedRef<FPointBubblesProxy> FPointBubblesProxy::CreateDemo()
{
	TSharedRef<FPointBubblesProxy> Proxy = MakeShared<FPointBubblesProxy>();

	TArray<FVector2D> DemoPoints;
	DemoPoints.Add(FVector2D(0.0f, 10.0f));
	DemoPoints.Add(FVector2D(1.0f, 18.0f));
	DemoPoints.Add(FVector2D(2.0f, 12.0f));
	DemoPoints.Add(FVector2D(3.0f, 26.0f));
	DemoPoints.Add(FVector2D(4.0f, 20.0f));
	DemoPoints.Add(FVector2D(5.0f, 32.0f));
	DemoPoints.Add(FVector2D(6.0f, 28.0f));
	Proxy->SetPoints(DemoPoints);

	return Proxy;
}

const FPointBubblesState& FPointBubblesProxy::GetState() const
{
	return State;
}

FPointBubblesProxy::FOnStateChanged& FPointBubblesProxy::OnStateChanged()
{
	return StateChanged;
}

void FPointBubblesProxy::SetPoints(const TArray<FVector2D>& InPoints)
{
	State.Points = InPoints;
	NChartMathLib::SortPointsByX(State.Points);
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetPadding(const FVector2D& InPadding)
{
	State.Padding = InPadding;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetPointRadius(float InRadius)
{
	State.PointRadius = FMath::Max(1.0f, InRadius);
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetActivationDistance(float InDistance)
{
	State.ActivationDistance = FMath::Max(1.0f, InDistance);
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetBreathAmplitude(float InAmplitude)
{
	State.BreathAmplitude = FMath::Max(0.0f, InAmplitude);
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetBreathSpeed(float InSpeed)
{
	State.BreathSpeed = FMath::Max(0.1f, InSpeed);
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetPointColor(const FLinearColor& InColor)
{
	State.PointColor = InColor;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetHoverPointColor(const FLinearColor& InColor)
{
	State.HoverPointColor = InColor;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetBubbleBackgroundColor(const FLinearColor& InColor)
{
	State.BubbleBackgroundColor = InColor;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetBubbleTextColor(const FLinearColor& InColor)
{
	State.BubbleTextColor = InColor;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetBubbleOffset(const FVector2D& InOffset)
{
	State.BubbleOffset = InOffset;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::SetHoverState(int32 InIndex, const FVector2D& InDataPoint, const FVector2D& InScreenPoint)
{
	State.bHasHover = true;
	State.HoveredIndex = InIndex;
	State.HoveredDataPoint = InDataPoint;
	State.HoveredScreenPoint = InScreenPoint;
	StateChanged.Broadcast();
}

void FPointBubblesProxy::ClearHoverState()
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
