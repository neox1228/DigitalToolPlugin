// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/AxisX/AxisXWidget.h"

#include "Core/NChartCartesianScale.h"
#include "Features/AxisX/AxisXProxy.h"
#include "Features/AxisX/AxisXState.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SAxisXWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = MakeShared<FAxisXProxy>();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &SAxisXWidget::HandleStateChanged);
}

SAxisXWidget::~SAxisXWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 SAxisXWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FAxisXState& State = Proxy->GetState();
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
	AxisPoints.Add(FVector2D(DrawMax.X, DrawMax.Y));

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
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		for (const FNChartAxisTick& Tick : Scale->XTicks)
		{
			TArray<FVector2D> TickLine;
			TickLine.Add(FVector2D(Tick.ScreenPos, DrawMax.Y));
			TickLine.Add(FVector2D(Tick.ScreenPos, DrawMax.Y + 4.0f));
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
						FVector2D(Tick.ScreenPos - LabelSize.X * 0.5f, DrawMax.Y + 6.0f),
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

FVector2D SAxisXWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void SAxisXWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
