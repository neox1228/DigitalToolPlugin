// Fill out your copyright notice in the Description page of Project Settings.


#include "Layout/ChartLayout.h"

#include "Core/ChartPaintContext.h"
#include "Styling/CoreStyle.h"

namespace
{
	constexpr double AxisLabelGap = 6.0;
	constexpr double AxisTitleGap = 4.0;
	constexpr double TitleGap = 8.0;
	constexpr double LegendGap = 10.0;
	constexpr double LegendMarkerSize = 10.0;
	constexpr double LegendMarkerGap = 5.0;
	constexpr double LegendItemGap = 18.0;
	constexpr double LegendRowPadding = 4.0;
	constexpr double MinPlotSize = 8.0;

	FVector2D DefaultMeasure(const FString& Text, const FSlateFontInfo& Font)
	{
		return FChartPaintContext::MeasureText(Text, Font);
	}

	FSlateFontInfo ResolveFont(const FSlateFontInfo& Candidate, float FallbackSize)
	{
		if (Candidate.HasValidFont())
		{
			return Candidate;
		}
		return FCoreStyle::GetDefaultFontStyle("Regular", FallbackSize);
	}

	int32 CountLegendSeries(const FChartSpec& Spec)
	{
		int32 Count = 0;
		for (const FChartSeriesSpec& Item : Spec.Series)
		{
			// 注意：这里不筛 bVisible —— 被隐藏的系列仍要留在图例里，否则点不回来
			if (Item.Style.bShowInLegend)
			{
				++Count;
			}
		}
		return Count;
	}

	void ApplyTickSuffix(FChartTickSet& TickSet, const FString& Suffix)
	{
		if (Suffix.IsEmpty())
		{
			return;
		}

		// 只给主刻度加后缀，次刻度不显示标签
		for (FChartTick& Tick : TickSet.Ticks)
		{
			if (Tick.bMajor && !Tick.Label.IsEmpty())
			{
				Tick.Label += Suffix;
			}
		}
	}

	void BuildLegendItems(FChartLayout& Layout, const FChartSpec& Spec, const FChartMeasureFunc& Measure)
	{
		Layout.LegendItems.Reset();
		if (!Layout.bHasLegend || Layout.LegendRect.GetSize().X <= 0.0)
		{
			return;
		}

		const double LineHeight = Measure(TEXT("Ag"), Layout.LegendFont).Y;
		const double RowHeight = FMath::Max(LineHeight + LegendRowPadding, LegendMarkerSize + 4.0);
		const bool bVertical = (Layout.LegendPosition == EChartLegendPosition::Right);

		double CursorX = Layout.LegendRect.Min.X;
		double CursorY = Layout.LegendRect.Min.Y;

		for (const FChartSeriesSpec& Item : Spec.Series)
		{
			if (!Item.Style.bShowInLegend)
			{
				continue;
			}

			const FVector2D LabelSize = Measure(Item.GetLegendLabel(), Layout.LegendFont);
			const double ItemWidth = LegendMarkerSize + LegendMarkerGap + LabelSize.X;

			FChartLegendItem Entry;
			Entry.SeriesKey = Item.SeriesKey;
			Entry.Rect = FBox2D(FVector2D(CursorX, CursorY),
			                    FVector2D(CursorX + ItemWidth, CursorY + RowHeight));
			Entry.MarkerRect = FBox2D(
				FVector2D(CursorX, CursorY + (RowHeight - LegendMarkerSize) * 0.5),
				FVector2D(CursorX + LegendMarkerSize, CursorY + (RowHeight + LegendMarkerSize) * 0.5));

			Layout.LegendItems.Add(Entry);

			if (bVertical)
			{
				CursorY += RowHeight;
			}
			else
			{
				CursorX += ItemWidth + LegendItemGap;
			}
		}
	}

	void MeasureMajorLabels(const FChartTickSet& TickSet, const FSlateFontInfo& Font,
	                        const FChartMeasureFunc& Measure, FVector2D& OutMaxSize)
	{
		OutMaxSize = FVector2D::ZeroVector;
		for (const FChartTick& Tick : TickSet.Ticks)
		{
			if (!Tick.bMajor || Tick.Label.IsEmpty())
			{
				continue;
			}

			const FVector2D Size = Measure(Tick.Label, Font);
			OutMaxSize.X = FMath::Max(OutMaxSize.X, Size.X);
			OutMaxSize.Y = FMath::Max(OutMaxSize.Y, Size.Y);
		}
	}
}

FText FChartLayoutBuilder::ComposeAxisTitle(const FChartAxisSpec& AxisSpec)
{
	if (AxisSpec.Unit.IsEmpty())
	{
		return AxisSpec.AxisTitle;
	}

	if (AxisSpec.AxisTitle.IsEmpty())
	{
		return FText::FromString(AxisSpec.Unit);
	}

	return FText::FromString(FString::Printf(TEXT("%s (%s)"), *AxisSpec.AxisTitle.ToString(), *AxisSpec.Unit));
}

FChartLayout FChartLayoutBuilder::Build(const FChartSpec& Spec,
                                        const FChartDomain& XDomain,
                                        const FChartDomain& YDomain,
                                        const FVector2D& WidgetSize,
                                        const FChartMeasureFunc& InMeasure)
{
	FChartLayout Layout;
	Layout.WidgetSize = WidgetSize;
	Layout.LegendPosition = Spec.Legend.Position;

	if (WidgetSize.X <= 1.0 || WidgetSize.Y <= 1.0)
	{
		return Layout;
	}

	const FChartMeasureFunc& Measure = InMeasure ? InMeasure : FChartMeasureFunc(&DefaultMeasure);

	Layout.AxisLabelFont = ResolveFont(Spec.AxisLabelFont, 9.0f);
	Layout.TitleFont = ResolveFont(Spec.Title.Font, 14.0f);
	Layout.LegendFont = ResolveFont(Spec.Legend.Font, 9.0f);

	// ------------------------------------------------------------------ 内容区
	const FMargin& Padding = Spec.Padding;
	FVector2D ContentMin(Padding.Left, Padding.Top);
	FVector2D ContentMax(WidgetSize.X - Padding.Right, WidgetSize.Y - Padding.Bottom);
	if (ContentMax.X - ContentMin.X < MinPlotSize || ContentMax.Y - ContentMin.Y < MinPlotSize)
	{
		return Layout;
	}

	// ------------------------------------------------------------------ 图表标题
	if (Spec.Title.IsVisible())
	{
		const FVector2D TitleSize = Measure(Spec.Title.Text.ToString(), Layout.TitleFont);
		Layout.TitleRect = FBox2D(ContentMin, FVector2D(ContentMax.X, ContentMin.Y + TitleSize.Y));
		ContentMin.Y += TitleSize.Y + TitleGap;
	}

	// ------------------------------------------------------------------ 图例（上 / 下）
	const bool bWantLegend = Spec.Legend.bShow && CountLegendSeries(Spec) > 0;
	if (bWantLegend && Spec.Legend.Position != EChartLegendPosition::Right)
	{
		const double LineHeight = Measure(TEXT("Ag"), Layout.LegendFont).Y;
		const double LegendHeight = LineHeight + LegendRowPadding;
		if (LegendHeight > 0.0)
		{
			if (Spec.Legend.Position == EChartLegendPosition::Top)
			{
				Layout.LegendRect = FBox2D(ContentMin, FVector2D(ContentMax.X, ContentMin.Y + LegendHeight));
				ContentMin.Y += LegendHeight + LegendGap;
			}
			else
			{
				Layout.LegendRect = FBox2D(FVector2D(ContentMin.X, ContentMax.Y - LegendHeight), ContentMax);
				ContentMax.Y -= LegendHeight + LegendGap;
			}
			Layout.bHasLegend = true;
		}
	}

	// ------------------------------------------------------------------ 刻度
	Layout.XAxis.TickSet = ChartTicks::MakeLinearTicks(XDomain, Spec.AxisX.DesiredTickCount, Spec.AxisX.Format, true);
	Layout.YAxis.TickSet = ChartTicks::MakeLinearTicks(YDomain, Spec.AxisY.DesiredTickCount, Spec.AxisY.Format, true);

	// 后缀必须在测量之前加，否则标签宽度会算窄、右边被裁
	ApplyTickSuffix(Layout.XAxis.TickSet, Spec.AxisX.TickSuffix);
	ApplyTickSuffix(Layout.YAxis.TickSet, Spec.AxisY.TickSuffix);

	MeasureMajorLabels(Layout.XAxis.TickSet, Layout.AxisLabelFont, Measure, Layout.XAxis.MaxLabelSize);
	MeasureMajorLabels(Layout.YAxis.TickSet, Layout.AxisLabelFont, Measure, Layout.YAxis.MaxLabelSize);

	// ------------------------------------------------------------------ 轴标题
	Layout.XAxis.Title = ComposeAxisTitle(Spec.AxisX);
	Layout.YAxis.Title = ComposeAxisTitle(Spec.AxisY);

	if (!Layout.XAxis.Title.IsEmpty())
	{
		Layout.XAxis.TitleSize = Measure(Layout.XAxis.Title.ToString(), Layout.AxisLabelFont);
	}
	if (!Layout.YAxis.Title.IsEmpty())
	{
		Layout.YAxis.TitleSize = Measure(Layout.YAxis.Title.ToString(), Layout.AxisLabelFont);
	}

	// ------------------------------------------------------------------ Y 轴标题带（绘图区上方）
	if (!Layout.YAxis.Title.IsEmpty() && Layout.YAxis.TitleSize.Y > 0.0)
	{
		Layout.YAxisTitleRect = FBox2D(ContentMin, FVector2D(ContentMax.X, ContentMin.Y + Layout.YAxis.TitleSize.Y));
		ContentMin.Y += Layout.YAxis.TitleSize.Y + AxisTitleGap;
	}

	// ------------------------------------------------------------------ 为轴预留的边距
	double XReserved = 0.0;
	if (Spec.AxisX.bShowLabels && Layout.XAxis.MaxLabelSize.Y > 0.0)
	{
		XReserved += Layout.XAxis.MaxLabelSize.Y + AxisLabelGap;
	}
	if (!Layout.XAxis.Title.IsEmpty())
	{
		XReserved += Layout.XAxis.TitleSize.Y + AxisTitleGap;
	}
	Layout.XAxis.ReservedSize = XReserved;

	double YReserved = 0.0;
	if (Spec.AxisY.bShowLabels && Layout.YAxis.MaxLabelSize.X > 0.0)
	{
		YReserved += Layout.YAxis.MaxLabelSize.X + AxisLabelGap;
	}
	Layout.YAxis.ReservedSize = YReserved;

	// ------------------------------------------------------------------ 绘图区
	FVector2D PlotMin(ContentMin.X + YReserved, ContentMin.Y);
	FVector2D PlotMax(ContentMax.X, ContentMax.Y - XReserved);

	// ------------------------------------------------------------------ 图例（右）
	if (bWantLegend && Spec.Legend.Position == EChartLegendPosition::Right)
	{
		double LegendWidth = 0.0;
		for (const FChartSeriesSpec& Item : Spec.Series)
		{
			if (!Item.bVisible || !Item.Style.bShowInLegend)
			{
				continue;
			}
			LegendWidth = FMath::Max(LegendWidth, Measure(Item.GetLegendLabel(), Layout.LegendFont).X);
		}

		if (LegendWidth > 0.0)
		{
			const double Total = LegendWidth + LegendMarkerSize + LegendMarkerGap + LegendGap;
			Layout.LegendRect = FBox2D(FVector2D(PlotMax.X - Total, PlotMin.Y), PlotMax);
			PlotMax.X -= Total;
			Layout.bHasLegend = true;
		}
	}

	if (PlotMax.X - PlotMin.X < MinPlotSize || PlotMax.Y - PlotMin.Y < MinPlotSize)
	{
		return Layout;
	}

	// ------------------------------------------------------------------ X 轴标题带（标签下方）
	if (!Layout.XAxis.Title.IsEmpty())
	{
		const double LabelBand = (Spec.AxisX.bShowLabels && Layout.XAxis.MaxLabelSize.Y > 0.0)
			? Layout.XAxis.MaxLabelSize.Y + AxisLabelGap
			: 0.0;
		const double Top = PlotMax.Y + LabelBand;
		Layout.XAxisTitleRect = FBox2D(FVector2D(PlotMin.X, Top),
		                               FVector2D(PlotMax.X, Top + Layout.XAxis.TitleSize.Y));
	}

	Layout.PlotRect = FBox2D(PlotMin, PlotMax);
	Layout.bValid = true;

	// 图例条目矩形必须在绘图区确定之后算，且与绘制共用同一份结果
	BuildLegendItems(Layout, Spec, Measure);

	return Layout;
}
