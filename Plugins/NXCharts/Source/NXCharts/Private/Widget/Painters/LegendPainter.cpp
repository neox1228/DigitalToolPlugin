#include "Widget/Painters/LegendPainter.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"

namespace
{
	
constexpr double LegendMarkerSize = 10.0;
	
constexpr double LegendMarkerGap = 5.0;

void PaintLegendImpl(const FChartPaintArgs& Args)
{
	auto& Ctx = Args.Ctx;
	const FChartFrame& Frame = Args.Frame;
	const FChartLayout& Layout = Frame.Layout;
	if (!Layout.bHasLegend || Layout.LegendItems.Num() == 0)
	{
		return;
	}

	const FLinearColor& BaseLabelColor = Frame.Spec.Legend.TextColor;

	for (const FChartLegendItem& Entry : Layout.LegendItems)
	{
		const FChartSeriesSpec* SeriesSpec = Frame.Spec.FindSeries(Entry.SeriesKey);
		if (!SeriesSpec)
		{
			continue;
		}

		// 隐藏的系列在图例里变暗，但仍然能点回来
		const float Alpha = SeriesSpec->bVisible ? 1.0f : 0.35f;

		FLinearColor SwatchColor = SeriesSpec->Style.Color;
		SwatchColor.A *= Alpha;

		FLinearColor TextColor = BaseLabelColor;
		TextColor.A *= Alpha;

		Ctx.DrawRect(Entry.MarkerRect.Min, Entry.MarkerRect.GetSize(), SwatchColor);
		Ctx.DrawText(SeriesSpec->GetLegendLabel(),
		             FVector2D(Entry.MarkerRect.Max.X + LegendMarkerGap, Entry.Rect.Min.Y + 2.0),
		             Layout.LegendFont, TextColor, EChartTextHAlign::Left, EChartTextVAlign::Top);
	}
}
}

EChartLayer FLegendPainter::GetLayer() const { return EChartLayer::Overlay; }

bool FLegendPainter::IsEnabled(const FChartPaintArgs& Args) const
{
	return Args.Frame.bValid && Args.Frame.Layout.bHasLegend && Args.Frame.Spec.Legend.bShow;
}

void FLegendPainter::Paint(const FChartPaintArgs& Args) const { PaintLegendImpl(Args); }