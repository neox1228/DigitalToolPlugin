#include "Widget/Painters/AnnotationPainter.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"

EChartLayer FAnnotationPainter::GetLayer() const
{
	return EChartLayer::Annotation;
}

bool FAnnotationPainter::IsEnabled(const FChartPaintArgs& Args) const
{
	return Args.Frame.bValid && Args.Frame.Spec.Annotations.Num() > 0;
}

void FAnnotationPainter::Paint(const FChartPaintArgs& Args) const
{
	FChartPaintContext& Ctx = Args.Ctx;
	const FChartFrame& Frame = Args.Frame;
	const FBox2D& Plot = Frame.Layout.PlotRect;
	const FSlateFontInfo& Font = Frame.Layout.AxisLabelFont;

	for (const FChartAnnotationSpec& Annotation : Frame.Spec.Annotations)
	{
		const float Thickness = FMath::Max(1.0f, Annotation.Thickness);

		if (Annotation.Kind == EChartAnnotationKind::Band)
		{
			const double LowValue = FMath::Min(Annotation.Value, Annotation.RangeEnd);
			const double HighValue = FMath::Max(Annotation.Value, Annotation.RangeEnd);

			double Top = Frame.Space.DataYToLocal(HighValue);
			double Bottom = Frame.Space.DataYToLocal(LowValue);
			Top = FMath::Clamp(Top, Plot.Min.Y, Plot.Max.Y);
			Bottom = FMath::Clamp(Bottom, Plot.Min.Y, Plot.Max.Y);
			if (Bottom <= Top)
			{
				continue;
			}

			FLinearColor BandColor = Annotation.Color;
			BandColor.A *= 0.25f;
			Ctx.DrawRect(FVector2D(Plot.Min.X, Top), FVector2D(Plot.Max.X, Bottom), BandColor);
			continue;
		}

		if (Annotation.Kind == EChartAnnotationKind::HorizontalLine)
		{
			const double Y = Frame.Space.DataYToLocal(Annotation.Value);
			if (Y < Plot.Min.Y || Y > Plot.Max.Y)
			{
				continue;
			}

			const FVector2D From(Plot.Min.X, Y);
			const FVector2D To(Plot.Max.X, Y);
			if (Annotation.bDashed)
			{
				Ctx.DrawDashedLine(From, To, 6.0f, 4.0f, Annotation.Color, Thickness);
			}
			else
			{
				Ctx.DrawLine(TArray<FVector2d>({ FVector2d(From), FVector2d(To) }), Annotation.Color, Thickness, true);
			}

			if (!Annotation.Label.IsEmpty())
			{
				const FVector2D LabelSize = FChartPaintContext::MeasureText(Annotation.Label, Font);
				Ctx.DrawText(Annotation.Label,
				             FVector2D(Plot.Max.X - LabelSize.X - 4.0, Y - LabelSize.Y - 2.0),
				             Font, Annotation.Color, EChartTextHAlign::Left, EChartTextVAlign::Top);
			}
			continue;
		}

		// VerticalLine
		const double X = Frame.Space.DataXToLocal(Annotation.Value);
		if (X < Plot.Min.X || X > Plot.Max.X)
		{
			continue;
		}

		const FVector2D From(X, Plot.Min.Y);
		const FVector2D To(X, Plot.Max.Y);
		if (Annotation.bDashed)
		{
			Ctx.DrawDashedLine(From, To, 6.0f, 4.0f, Annotation.Color, Thickness);
		}
		else
		{
			Ctx.DrawLine(TArray<FVector2d>({ FVector2d(From), FVector2d(To) }), Annotation.Color, Thickness, true);
		}

		if (!Annotation.Label.IsEmpty())
		{
			Ctx.DrawText(Annotation.Label, FVector2D(X + 4.0, Plot.Min.Y + 4.0),
			             Font, Annotation.Color, EChartTextHAlign::Left, EChartTextVAlign::Top);
		}
	}
}