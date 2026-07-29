// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/LineSeries/LineSeriesWidget.h"

#include "Core/NChartCartesianScale.h"
#include "Core/NChartLayoutUtils.h"
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

	if (State.Points.Num() < 2)
	{
		return LayerId;
	}

	TArray<FVector2D> LinePoints;
	const TSharedPtr<FNChartCartesianScale> Scale = Proxy->GetCartesianScale();
	if (Scale.IsValid())
	{
		Scale->UpdatePixelRect(Scale->Padding, Size);
		Scale->BuildScreenPoints(State.Points, LinePoints);
	}
	else
	{
		FNChartScreenLayout Layout;
		if (!FNChartLayoutUtils::BuildScreenLayout(State.Points, State.Padding, Size, 2, Layout))
		{
			return LayerId;
		}
		LinePoints = Layout.ScreenPoints;
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
