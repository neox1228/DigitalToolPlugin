// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/AxisY/AxisYWidget.h"

#include "Core/NChartCartesianScale.h"
#include "Features/AxisY/AxisYProxy.h"
#include "Features/AxisY/AxisYState.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SAxisYWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = MakeShared<FAxisYProxy>();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &SAxisYWidget::HandleStateChanged);
}

SAxisYWidget::~SAxisYWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 SAxisYWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FAxisYState& State = Proxy->GetState();
	const FVector2D Size = FVector2D(AllottedGeometry.GetLocalSize());
	const TSharedPtr<FNChartCartesianScale> Scale = Proxy->GetCartesianScale();

	FVector2D DrawMin = State.Padding;
	FVector2D DrawMax = FVector2D(Size.X - State.Padding.X, Size.Y - State.Padding.Y);
	if (Scale.IsValid())
	{
		Scale->UpdatePixelRect(State.Padding, Size);
		DrawMin = Scale->DrawMin;
		DrawMax = Scale->DrawMax;
	}

	TArray<FVector2D> AxisPoints;
	AxisPoints.Add(FVector2D(DrawMin.X, DrawMax.Y));
	AxisPoints.Add(FVector2D(DrawMin.X, DrawMin.Y));

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		AxisPoints,
		ESlateDrawEffect::None,
		State.AxisColor,
		true,
		State.AxisThickness);

	if (Scale.IsValid() && State.bShowTicks)
	{
		for (const FNChartAxisTick& Tick : Scale->YTicks)
		{
			TArray<FVector2D> TickLine;
			TickLine.Add(FVector2D(DrawMin.X - 4.0f, Tick.ScreenPos));
			TickLine.Add(FVector2D(DrawMin.X, Tick.ScreenPos));
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				TickLine,
				ESlateDrawEffect::None,
				State.AxisColor,
				true,
				1.0f);

			if (State.bShowLabels)
			{
				const FVector2D LabelSize(48.0f, 14.0f);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 2,
					AllottedGeometry.ToPaintGeometry(
						FVector2D(DrawMin.X - LabelSize.X - 4.0f, Tick.ScreenPos - LabelSize.Y * 0.5f),
						LabelSize),
					Tick.Label,
					FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 9),
					ESlateDrawEffect::None,
					FLinearColor::Gray);
			}
		}
	}

	return LayerId + 2;
}

FVector2D SAxisYWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void SAxisYWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
