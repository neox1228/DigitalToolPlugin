// Copyright NCharts Plugin. All Rights Reserved.
// SPointBubblesWidget 实现：散点圆绘制、Hover 呼吸动画与气泡提示

#include "Features/PointBubbles/PointBubblesWidget.h"

#include "Core/NChartCartesianScale.h"
#include "Core/NChartLayoutUtils.h"

#include "Features/PointBubbles/PointBubblesProxy.h"
#include "Features/PointBubbles/PointBubblesState.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SWidget.h"

namespace
{
	/** 在屏幕坐标点集中查找距离鼠标最近的点 */
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

	// Hover 时计算呼吸动画偏移量（正弦波）
	const double TimeSeconds = FPlatformTime::Seconds();
	const bool bAnimateHover = State.bHasHover && State.HoveredIndex != INDEX_NONE;
	const float Breath = bAnimateHover
		? FMath::Sin(static_cast<float>(TimeSeconds) * State.BreathSpeed) * State.BreathAmplitude
		: 0.0f;

	// 绘制所有散点圆
	for (int32 Index = 0; Index < ScreenPoints.Num(); ++Index)
	{
		const bool bHovered = bAnimateHover && Index == State.HoveredIndex;
		const float Radius = State.PointRadius + (bHovered ? Breath : 0.0f);
		const FLinearColor Color = bHovered ? State.HoverPointColor : State.PointColor;
		DrawCircle(AllottedGeometry, OutDrawElements, LayerId + 1, ScreenPoints[Index], Radius, Color);
	}

	// Hover 时绘制数值气泡
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

	// 持续触发重绘以更新呼吸动画帧
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

	// 启动呼吸动画定时器
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

	const TSharedPtr<FNChartCartesianScale> Scale = Proxy->GetCartesianScale();
	if (Scale.IsValid())
	{
		Scale->UpdatePixelRect(Scale->Padding, GeometrySize);
		Scale->BuildScreenPoints(State.Points, OutScreenPoints);
		return OutScreenPoints.Num() > 0;
	}

	FNChartScreenLayout Layout;
	if (!FNChartLayoutUtils::BuildScreenLayout(State.Points, State.Padding, GeometrySize, 1, Layout))
	{
		return false;
	}

	OutScreenPoints = Layout.ScreenPoints;
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
	// 使用多层同心圆环线段填充，近似绘制实心圆
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
