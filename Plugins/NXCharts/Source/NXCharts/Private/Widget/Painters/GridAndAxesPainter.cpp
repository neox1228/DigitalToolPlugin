#include "Widget/Painters/GridAndAxesPainter.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"
#include "Layout/ChartTicks.h"

namespace
{
	
constexpr float AxisLineThickness = 1.0f;
	
constexpr double XLabelGap = 5.0;
	
constexpr double YLabelGap = 7.0;

void PaintGridAndAxesImpl(const FChartPaintArgs& Args)
{
	auto& Ctx = Args.Ctx;
	const FChartFrame& Frame = Args.Frame;
	const FChartLayout& Layout = Frame.Layout;
	const FBox2D& Plot = Layout.PlotRect;

	// 颜色全部来自 Spec，画布不再持有任何硬编码配色
	const FChartSpec& Opt = Frame.Spec;
	const FLinearColor& AxisColor = Opt.Colors.AxisColor;
	const FLinearColor& LabelColor = Opt.Colors.LabelColor;
	const FLinearColor& TitleColor = Opt.Colors.TitleColor;

	auto DrawGridSegment = [&Ctx, &Opt](const FVector2D& From, const FVector2D& To)
	{
		if (Opt.Grid.bDashed)
		{
			Ctx.DrawDashedLine(From, To, Opt.Grid.DashLength, Opt.Grid.GapLength, Opt.Grid.Color, 1.0f);
		}
		else
		{
			Ctx.DrawLine(TArray<FVector2d>({ FVector2d(From), FVector2d(To) }), Opt.Grid.Color, 1.0f, false);
		}
	};

	// ---- 网格（裁剪在绘图区内）
	Ctx.PushClip(Plot);
	{
		for (const FChartTick& Tick : Layout.XAxis.TickSet.Ticks)
		{
			if (!Tick.bMajor)
			{
				continue;
			}
			const double X = Frame.Space.DataXToLocal(Tick.Value);
			DrawGridSegment(FVector2D(X, Plot.Min.Y), FVector2D(X, Plot.Max.Y));
		}

		for (const FChartTick& Tick : Layout.YAxis.TickSet.Ticks)
		{
			if (!Tick.bMajor)
			{
				continue;
			}
			const double Y = Frame.Space.DataYToLocal(Tick.Value);
			DrawGridSegment(FVector2D(Plot.Min.X, Y), FVector2D(Plot.Max.X, Y));
		}
	}
	Ctx.PopClip();

	// ---- 轴线
	Ctx.DrawLine(TArray<FVector2d>({ FVector2d(Plot.Min.X, Plot.Min.Y), FVector2d(Plot.Min.X, Plot.Max.Y) }),
	             AxisColor, AxisLineThickness, false);
	Ctx.DrawLine(TArray<FVector2d>({ FVector2d(Plot.Min.X, Plot.Max.Y), FVector2d(Plot.Max.X, Plot.Max.Y) }),
	             AxisColor, AxisLineThickness, false);

	// ---- 刻度标签
	if (Frame.Spec.AxisX.bShowLabels)
	{
		for (const FChartTick& Tick : Layout.XAxis.TickSet.Ticks)
		{
			if (!Tick.bMajor || Tick.Label.IsEmpty())
			{
				continue;
			}
			const double X = Frame.Space.DataXToLocal(Tick.Value);
			Ctx.DrawText(Tick.Label, FVector2D(X, Plot.Max.Y + XLabelGap), Layout.AxisLabelFont, LabelColor,
			             EChartTextHAlign::Center, EChartTextVAlign::Top);
		}
	}

	if (Frame.Spec.AxisY.bShowLabels)
	{
		for (const FChartTick& Tick : Layout.YAxis.TickSet.Ticks)
		{
			if (!Tick.bMajor || Tick.Label.IsEmpty())
			{
				continue;
			}
			const double Y = Frame.Space.DataYToLocal(Tick.Value);
			Ctx.DrawText(Tick.Label, FVector2D(Plot.Min.X - YLabelGap, Y), Layout.AxisLabelFont, LabelColor,
			             EChartTextHAlign::Right, EChartTextVAlign::Middle);
		}
	}

	// ---- 标题
	if (Frame.Spec.Title.IsVisible())
	{
		Ctx.DrawText(Frame.Spec.Title.Text.ToString(), Layout.TitleRect.Min, Layout.TitleFont, TitleColor,
		             EChartTextHAlign::Left, EChartTextVAlign::Top);
	}

	if (!Layout.YAxis.Title.IsEmpty())
	{
		Ctx.DrawText(Layout.YAxis.Title.ToString(), Layout.YAxisTitleRect.Min, Layout.AxisLabelFont, LabelColor,
		             EChartTextHAlign::Left, EChartTextVAlign::Top);
	}

	if (!Layout.XAxis.Title.IsEmpty())
	{
		const double CenterX = (Layout.XAxisTitleRect.Min.X + Layout.XAxisTitleRect.Max.X) * 0.5;
		Ctx.DrawText(Layout.XAxis.Title.ToString(), FVector2D(CenterX, Layout.XAxisTitleRect.Min.Y),
		             Layout.AxisLabelFont, LabelColor, EChartTextHAlign::Center, EChartTextVAlign::Top);
	}
}
}

EChartLayer FGridAndAxesPainter::GetLayer() const { return EChartLayer::Grid; }
void FGridAndAxesPainter::Paint(const FChartPaintArgs& Args) const { PaintGridAndAxesImpl(Args); }