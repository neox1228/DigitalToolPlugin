#include "Widget/Painters/BackgroundPainter.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"

namespace
{
	void PaintBackgroundImpl(const FChartPaintArgs& Args)
	{
		const FChartBackgroundSpec& Bg = Args.Frame.Spec.Background;
		if (Bg.bGradient)
		{
			Args.Ctx.DrawVerticalGradient(FBox2D(FVector2D::ZeroVector, Args.LocalSize), Bg.Color, Bg.ColorBottom);
		}
		else
		{
			Args.Ctx.DrawRect(FVector2D::ZeroVector, Args.LocalSize, Bg.Color);
		}
	}
}

EChartLayer FBackgroundPainter::GetLayer() const { return EChartLayer::Background; }
void FBackgroundPainter::Paint(const FChartPaintArgs& Args) const { PaintBackgroundImpl(Args); }