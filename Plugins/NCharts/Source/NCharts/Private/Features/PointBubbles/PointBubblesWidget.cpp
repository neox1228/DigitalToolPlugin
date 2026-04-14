#include "Features/PointBubbles/PointBubblesWidget.h"

#include "Features/PointBubbles/PointBubblesProxy.h"
#include "Features/PointBubbles/PointBubblesState.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SWidget.h"

namespace
{
	bool FindNearestBubblePoint(
		const TArray<FVector2D>& ScreenPoints,
		const FVector2D& MousePosition,
		int32& OutIndex,
		float& OutDistanceSquared)
	{
		if (ScreenPoints.Num() == 0)
		{
			return false;
		}

		OutIndex = INDEX_NONE;
		OutDistanceSquared = TNumericLimits<float>::Max();

		for (int32 Index = 0; Index < ScreenPoints.Num(); ++Index)
		{
			const float DistanceSquared = FVector2D::DistSquared(ScreenPoints[Index], MousePosition);
			if (DistanceSquared < OutDistanceSquared)
			{
				OutDistanceSquared = DistanceSquared;
				OutIndex = Index;
			}
		}

		return OutIndex != INDEX_NONE;
	}
}

void SPointBubblesWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = FPointBubblesProxy::CreateDemo();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &SPointBubblesWidget::HandleStateChanged);
}

SPointBubblesWidget::~SPointBubblesWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 SPointBubblesWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	if (!Proxy.IsValid())
	{
		return LayerId;
	}

	const FPointBubblesState& State = Proxy->GetState();
	TArray<FVector2D> ScreenPoints;
	if (!BuildLayout(FVector2D(AllottedGeometry.GetLocalSize()), ScreenPoints))
	{
		return LayerId;
	}

	const double TimeSeconds = FPlatformTime::Seconds();
	const bool bAnimateHover = State.bHasHover && State.HoveredIndex != INDEX_NONE;
	const float Breath = bAnimateHover
		? FMath::Sin(static_cast<float>(TimeSeconds) * State.BreathSpeed) * State.BreathAmplitude
		: 0.0f;

	for (int32 Index = 0; Index < ScreenPoints.Num(); ++Index)
	{
		const bool bHovered = bAnimateHover && Index == State.HoveredIndex;
		const float Radius = State.PointRadius + (bHovered ? Breath : 0.0f);
		const FLinearColor Color = bHovered ? State.HoverPointColor : State.PointColor;
		DrawCircle(AllottedGeometry, OutDrawElements, LayerId + 1, ScreenPoints[Index], Radius, Color);
	}

	if (bAnimateHover && State.Points.IsValidIndex(State.HoveredIndex))
	{
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		const FVector2D BubbleSize(130.0f, 42.0f);
		const FVector2D BubblePos = FVector2D(
			State.HoveredScreenPoint.X - BubbleSize.X * 0.5f + State.BubbleOffset.X,
			State.HoveredScreenPoint.Y - BubbleSize.Y + State.BubbleOffset.Y);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(BubblePos, BubbleSize),
			WhiteBrush,
			ESlateDrawEffect::None,
			State.BubbleBackgroundColor);

		const FText Text = FText::Format(
			FText::FromString(TEXT("X: {0}\nY: {1}")),
			FText::AsNumber(State.HoveredDataPoint.X),
			FText::AsNumber(State.HoveredDataPoint.Y));

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 3,
			AllottedGeometry.ToPaintGeometry(FVector2D(BubblePos.X + 8.0f, BubblePos.Y + 6.0f), BubbleSize),
			Text,
			FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10),
			ESlateDrawEffect::None,
			State.BubbleTextColor);

		return LayerId + 3;
	}

	return LayerId + 1;
}

FReply SPointBubblesWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Proxy.IsValid())
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const bool bUpdated = UpdateHoverState(MyGeometry, LocalMousePosition);
	return bUpdated ? FReply::Handled() : FReply::Unhandled();
}

void SPointBubblesWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	ClearHoverState();
}

FVector2D SPointBubblesWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void SPointBubblesWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}

EActiveTimerReturnType SPointBubblesWidget::HandleBreathTick(double InCurrentTime, float InDeltaTime)
{
	if (!Proxy.IsValid())
	{
		return EActiveTimerReturnType::Stop;
	}

	const FPointBubblesState& State = Proxy->GetState();
	if (!State.bHasHover)
	{
		return EActiveTimerReturnType::Stop;
	}

	Invalidate(EInvalidateWidgetReason::Paint);
	return EActiveTimerReturnType::Continue;
}

bool SPointBubblesWidget::UpdateHoverState(const FGeometry& MyGeometry, const FVector2D& LocalMousePosition)
{
	const FPointBubblesState& State = Proxy->GetState();
	TArray<FVector2D> ScreenPoints;
	if (!BuildLayout(FVector2D(MyGeometry.GetLocalSize()), ScreenPoints))
	{
		ClearHoverState();
		return false;
	}

	int32 HoverIndex = INDEX_NONE;
	float BestDistanceSquared = 0.0f;
	if (!FindNearestBubblePoint(ScreenPoints, LocalMousePosition, HoverIndex, BestDistanceSquared) || !State.Points.IsValidIndex(HoverIndex))
	{
		ClearHoverState();
		return false;
	}

	if (BestDistanceSquared > FMath::Square(State.ActivationDistance))
	{
		ClearHoverState();
		return false;
	}

	Proxy->SetHoverState(HoverIndex, State.Points[HoverIndex], ScreenPoints[HoverIndex]);

	if (!BreathTimerHandle.IsValid())
	{
		BreathTimerHandle = RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &SPointBubblesWidget::HandleBreathTick));
	}

	Invalidate(EInvalidateWidgetReason::Paint);
	return true;
}

void SPointBubblesWidget::ClearHoverState()
{
	if (Proxy.IsValid())
	{
		Proxy->ClearHoverState();
	}
	BreathTimerHandle.Reset();
	Invalidate(EInvalidateWidgetReason::Paint);
}

bool SPointBubblesWidget::BuildLayout(const FVector2D& GeometrySize, TArray<FVector2D>& OutScreenPoints) const
{
	if (!Proxy.IsValid())
	{
		return false;
	}

	const FPointBubblesState& State = Proxy->GetState();
	if (State.Points.Num() <= 0)
	{
		return false;
	}

	const FVector2D DrawMin = State.Padding;
	const FVector2D DrawMax = FVector2D(GeometrySize.X - State.Padding.X, GeometrySize.Y - State.Padding.Y);
	const FVector2D DrawSize = DrawMax - DrawMin;
	if (DrawSize.X <= 1.0f || DrawSize.Y <= 1.0f)
	{
		return false;
	}

	FVector2D Min(FLT_MAX, FLT_MAX);
	FVector2D Max(-FLT_MAX, -FLT_MAX);
	for (const FVector2D& Point : State.Points)
	{
		Min.X = FMath::Min(Min.X, Point.X);
		Min.Y = FMath::Min(Min.Y, Point.Y);
		Max.X = FMath::Max(Max.X, Point.X);
		Max.Y = FMath::Max(Max.Y, Point.Y);
	}

	const float RangeX = FMath::Max(Max.X - Min.X, 1.0f);
	const float RangeY = FMath::Max(Max.Y - Min.Y, 1.0f);

	OutScreenPoints.Reset();
	OutScreenPoints.Reserve(State.Points.Num());

	for (const FVector2D& Point : State.Points)
	{
		const float NormalX = (Point.X - Min.X) / RangeX;
		const float NormalY = (Point.Y - Min.Y) / RangeY;
		OutScreenPoints.Add(FVector2D(
			DrawMin.X + NormalX * DrawSize.X,
			DrawMax.Y - NormalY * DrawSize.Y));
	}

	return true;
}

void SPointBubblesWidget::DrawCircle(
	const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FVector2D& Center,
	float Radius,
	const FLinearColor& Color) const
{
	const int32 SegmentCount = 20;
	for (float FillRadius = FMath::Max(1.0f, Radius); FillRadius >= 1.0f; FillRadius -= 1.0f)
	{
		TArray<FVector2D> RingPoints;
		RingPoints.Reserve(SegmentCount + 1);

		for (int32 Segment = 0; Segment <= SegmentCount; ++Segment)
		{
			const float Angle = (static_cast<float>(Segment) / static_cast<float>(SegmentCount)) * 2.0f * PI;
			RingPoints.Add(FVector2D(
				Center.X + FMath::Cos(Angle) * FillRadius,
				Center.Y + FMath::Sin(Angle) * FillRadius));
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			RingPoints,
			ESlateDrawEffect::None,
			Color,
			true,
			1.0f);
	}
}
