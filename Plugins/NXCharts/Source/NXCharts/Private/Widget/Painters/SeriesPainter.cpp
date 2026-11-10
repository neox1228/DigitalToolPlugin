#include "Widget/Painters/SeriesPainter.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"

namespace
{
void PaintSeriesImpl(const FChartPaintArgs& Args)
{
	auto& Ctx = Args.Ctx;
	const FChartFrame& Frame = Args.Frame;
	// 曲线必须裁剪在绘图区内，否则超出范围的数据会画到轴标签上
	Ctx.PushClip(Frame.Layout.PlotRect);
	{
		// 第一遍：面积填充。先全部画完，避免后画的填充盖住前一条系列的线
		for (const FChartSeriesGeometry& Series : Frame.Series)
		{
			if (!Series.bFillArea || Series.LocalPoints.Num() < 2)
			{
				continue;
			}

			const FLinearColor TopColor(Series.Color.R, Series.Color.G, Series.Color.B, Series.AreaFillOpacity);
			const FLinearColor BottomColor(Series.Color.R, Series.Color.G, Series.Color.B, 0.0f);
			Ctx.DrawGradientBand(Series.LocalPoints, Frame.Layout.PlotRect.Max.Y, TopColor, BottomColor);
		}

		// 第二遍：曲线与标记
		for (const FChartSeriesGeometry& Series : Frame.Series)
		{
			if (Series.LocalPoints.Num() < 2)
			{
				continue;
			}

			// 发光打底：粗线 + 低透明度，制造霓虹感
			if (Series.bGlow)
			{
				const FLinearColor GlowColor(Series.Color.R, Series.Color.G, Series.Color.B, 0.18f);
				Ctx.DrawLine(Series.LocalPoints, GlowColor, Series.GlowThickness, true);
			}

			Ctx.DrawLine(Series.LocalPoints, Series.Color, Series.LineThickness, Series.bAntialias);

			if (Series.bShowMarkers)
			{
				for (const FVector2d& Point : Series.LocalPoints)
				{
					Ctx.DrawCircle(Point, Series.MarkerSize, Series.Color, 1.5f, 10);
				}
			}
		}
	}
	Ctx.PopClip();
}
}

EChartLayer FSeriesPainter::GetLayer() const { return EChartLayer::Series; }
bool FSeriesPainter::IsEnabled(const FChartPaintArgs& Args) const { return Args.Frame.Series.Num() > 0; }
void FSeriesPainter::Paint(const FChartPaintArgs& Args) const { PaintSeriesImpl(Args); }