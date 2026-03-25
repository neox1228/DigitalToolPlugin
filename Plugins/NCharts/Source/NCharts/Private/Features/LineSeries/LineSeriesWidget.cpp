#include "Features/LineSeries/LineSeriesWidget.h"

#include "Features/LineSeries/LineSeriesProxy.h"
#include "Features/LineSeries/LineSeriesState.h"
#include "Rendering/DrawElements.h"

void SLineSeriesWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = FLineSeriesProxy::CreateDemo();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &SLineSeriesWidget::HandleStateChanged);
}

SLineSeriesWidget::~SLineSeriesWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 SLineSeriesWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FLineSeriesState& State = Proxy->GetState();
	const FVector2D Size = FVector2D(AllottedGeometry.GetLocalSize());
	const FVector2D Padding = State.Padding;
	const FVector2D DrawMin = Padding;
	const FVector2D DrawMax = FVector2D(Size.X - Padding.X, Size.Y - Padding.Y);

	if (State.Points.Num() < 2)
	{
		return LayerId;
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
	const FVector2D DrawSize = FVector2D(DrawMax.X - DrawMin.X, DrawMax.Y - DrawMin.Y);

	TArray<FVector2D> LinePoints;
	LinePoints.Reserve(State.Points.Num());
	for (const FVector2D& Point : State.Points)
	{
		const float NormalX = (Point.X - Min.X) / RangeX;
		const float NormalY = (Point.Y - Min.Y) / RangeY;

		const float X = DrawMin.X + NormalX * DrawSize.X;
		const float Y = DrawMax.Y - NormalY * DrawSize.Y;
		LinePoints.Add(FVector2D(X, Y));
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		LinePoints,
		ESlateDrawEffect::None,
		State.ActiveLineColor,
		true,
		State.LineThickness);

	return LayerId;
}

FVector2D SLineSeriesWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void SLineSeriesWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
