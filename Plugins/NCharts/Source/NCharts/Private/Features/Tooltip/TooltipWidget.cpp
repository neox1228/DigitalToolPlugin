#include "Features/Tooltip/TooltipWidget.h"

#include "Features/LineSeries/LineSeriesProxy.h"
#include "Features/LineSeries/LineSeriesState.h"
#include "Features/Tooltip/TooltipProxy.h"
#include "Features/Tooltip/TooltipState.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
	bool BuildLineSeriesLayout(
		const FLineSeriesState& State,
		const FVector2D& GeometrySize,
		FVector2D& OutDrawMin,
		FVector2D& OutDrawMax,
		TArray<FVector2D>& OutScreenPoints)
	{
		if (State.Points.Num() < 2)
		{
			return false;
		}

		OutDrawMin = State.Padding;
		OutDrawMax = FVector2D(GeometrySize.X - State.Padding.X, GeometrySize.Y - State.Padding.Y);
		const FVector2D DrawSize = OutDrawMax - OutDrawMin;
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
				OutDrawMin.X + NormalX * DrawSize.X,
				OutDrawMax.Y - NormalY * DrawSize.Y));
		}

		return true;
	}

	bool FindNearestPoint(
		const TArray<FVector2D>& ScreenPoints,
		const FVector2D& MousePosition,
		int32& OutIndex,
		float& OutDistance)
	{
		if (ScreenPoints.Num() == 0)
		{
			return false;
		}

		OutIndex = INDEX_NONE;
		OutDistance = TNumericLimits<float>::Max();

		for (int32 Index = 0; Index < ScreenPoints.Num(); ++Index)
		{
			const float Distance = FVector2D::DistSquared(ScreenPoints[Index], MousePosition);
			if (Distance < OutDistance)
			{
				OutDistance = Distance;
				OutIndex = Index;
			}
		}

		return OutIndex != INDEX_NONE;
	}
}

void STooltipWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = MakeShared<FTooltipProxy>();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &STooltipWidget::HandleStateChanged);
}

STooltipWidget::~STooltipWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 STooltipWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FTooltipState& TooltipState = Proxy->GetState();
	if (!TooltipState.bEnableTooltip || !TooltipState.bHasHover || TooltipState.HoveredIndex == INDEX_NONE)
	{
		return LayerId;
	}

	TSharedPtr<FLineSeriesProxy> TargetLineProxy = Proxy->GetTargetLineProxy();
	if (!TargetLineProxy.IsValid())
	{
		return LayerId;
	}

	const FLineSeriesState& LineState = TargetLineProxy->GetState();
	const FVector2D Size = FVector2D(AllottedGeometry.GetLocalSize());
	FVector2D DrawMin;
	FVector2D DrawMax;
	TArray<FVector2D> ScreenPoints;
	if (!BuildLineSeriesLayout(LineState, Size, DrawMin, DrawMax, ScreenPoints))
	{
		return LayerId;
	}

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FVector2D HoverPoint = TooltipState.HoveredScreenPoint;
	const FVector2D TooltipSize(140.0f, 42.0f);
	const FVector2D TooltipPos = HoverPoint + TooltipState.TooltipOffset;

	if (TooltipState.bShowHoverLine)
	{
		TArray<FVector2D> HoverLinePoints;
		HoverLinePoints.Add(FVector2D(HoverPoint.X, DrawMin.Y));
		HoverLinePoints.Add(FVector2D(HoverPoint.X, DrawMax.Y));
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			HoverLinePoints,
			ESlateDrawEffect::None,
			TooltipState.HoverLineColor,
			true,
			1.0f);
	}

	if (TooltipState.bShowHoverPoint)
	{
		const FVector2D MarkerSize(TooltipState.HoverPointRadius * 2.0f, TooltipState.HoverPointRadius * 2.0f);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(FVector2D(HoverPoint.X - TooltipState.HoverPointRadius, HoverPoint.Y - TooltipState.HoverPointRadius), MarkerSize),
			WhiteBrush,
			ESlateDrawEffect::None,
			TooltipState.HoverPointColor);
	}

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry.ToPaintGeometry(TooltipPos, TooltipSize),
		WhiteBrush,
		ESlateDrawEffect::None,
		TooltipState.TooltipBackgroundColor);

	const FText TooltipText = FText::Format(
		FText::FromString(TEXT("X: {0}\nY: {1}")),
		FText::AsNumber(TooltipState.HoveredDataPoint.X),
		FText::AsNumber(TooltipState.HoveredDataPoint.Y));

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId + 4,
		AllottedGeometry.ToPaintGeometry(FVector2D(TooltipPos.X + 8.0f, TooltipPos.Y + 6.0f), TooltipSize),
		TooltipText,
		FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10),
		ESlateDrawEffect::None,
		TooltipState.TooltipTextColor);

	return LayerId + 4;
}

FReply STooltipWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Proxy.IsValid())
	{
		return FReply::Unhandled();
	}

	const FTooltipState& TooltipState = Proxy->GetState();
	if (!TooltipState.bEnableTooltip)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	if (!UpdateHoverState(MyGeometry, LocalMousePosition))
	{
		return FReply::Unhandled();
	}

	Invalidate(EInvalidateWidgetReason::Paint);
	return FReply::Handled();
}

void STooltipWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	ClearHoverState();
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D STooltipWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void STooltipWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}

bool STooltipWidget::UpdateHoverState(const FGeometry& MyGeometry, const FVector2D& LocalMousePosition)
{
	TSharedPtr<FLineSeriesProxy> TargetLineProxy = Proxy->GetTargetLineProxy();
	if (!TargetLineProxy.IsValid())
	{
		ClearHoverState();
		return false;
	}

	const FLineSeriesState& LineState = TargetLineProxy->GetState();
	const FTooltipState& TooltipState = Proxy->GetState();
	FVector2D DrawMin;
	FVector2D DrawMax;
	TArray<FVector2D> ScreenPoints;
	if (!BuildLineSeriesLayout(LineState, FVector2D(MyGeometry.GetLocalSize()), DrawMin, DrawMax, ScreenPoints))
	{
		ClearHoverState();
		return false;
	}

	int32 HoverIndex = INDEX_NONE;
	float BestDistance = 0.0f;
	if (!FindNearestPoint(ScreenPoints, LocalMousePosition, HoverIndex, BestDistance) || !LineState.Points.IsValidIndex(HoverIndex))
	{
		ClearHoverState();
		return false;
	}

	if (BestDistance > FMath::Square(TooltipState.ActivationDistance))
	{
		ClearHoverState();
		return false;
	}

	Proxy->SetHoverState(HoverIndex, LineState.Points[HoverIndex], ScreenPoints[HoverIndex]);
	return true;
}

void STooltipWidget::ClearHoverState()
{
	if (Proxy.IsValid())
	{
		Proxy->ClearHoverState();
	}
}
