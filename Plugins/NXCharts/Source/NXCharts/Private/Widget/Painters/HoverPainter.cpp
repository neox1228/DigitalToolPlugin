#include "Widget/Painters/HoverPainter.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"
#include "Layout/ChartTicks.h"
#include "Widget/NXChartWidget.h"

namespace
{
	
constexpr double MaxTooltipNameWidth = 140.0;

	
FSlateFontInfo FitFontToWidth(const FSlateFontInfo& BaseFont, const FString& Text,
	                              double MaxWidth, double& OutWidth)
	{
		OutWidth = FChartPaintContext::MeasureText(Text, BaseFont).X;
		if (OutWidth <= MaxWidth || MaxWidth <= 1.0)
		{
			return BaseFont;
		}

		FSlateFontInfo Fitted = BaseFont;
		double Scale = MaxWidth / OutWidth;

		// 字号和宽度不是严格线性（受 hinting/行距影响），迭代几次收敛
		for (int32 Attempt = 0; Attempt < 3; ++Attempt)
		{
			Fitted.Size = FMath::Max(6.0f, static_cast<float>(Fitted.Size * Scale));
			OutWidth = FChartPaintContext::MeasureText(Text, Fitted).X;
			if (OutWidth <= MaxWidth)
			{
				break;
			}
			Scale = MaxWidth / OutWidth;
		}

		return Fitted;
	}

	
const FLinearColor CrosshairColor(0.55f, 0.62f, 0.72f, 0.75f);
	
const FLinearColor TooltipBackgroundColor(0.06f, 0.08f, 0.11f, 0.94f);

	
FString FormatValueWithStep(double Value, double Step, EChartTickFormat Format)
	{
		if (Step <= 0.0)
		{
			return FString::Printf(TEXT("%.2f"), Value);
		}
		return ChartTicks::FormatValue(Value, Step, Format);
	}

void PaintHoverImpl(const FChartPaintArgs& Args)
{
	auto& Ctx = Args.Ctx;
	const FChartFrame& Frame = Args.Frame;
	const bool bHoverActive = Args.bHoverActive;
	const FVector2D HoverLocal = Args.HoverLocal;
	UNXChartWidget* Widget = Args.Widget;
	const bool bExternal = (Widget != nullptr) && Widget->IsCrosshairActive();

	// 外部十字准星优先：激活期间鼠标悬停让位，避免两条竖线打架
	if ((!bExternal && !bHoverActive) || !Frame.bValid)
	{
		return;
	}

	const FChartLayout& Layout = Frame.Layout;
	const FBox2D& Plot = Layout.PlotRect;

	// 竖线位置：外部用数据 X 映射，鼠标用光标局部 X
	const double LocalX = bExternal ? Frame.Space.DataXToLocal(Widget->GetCrosshairX()) : HoverLocal.X;
	if (LocalX < Plot.Min.X || LocalX > Plot.Max.X)
	{
		return;
	}

	const FChartSpec& Opt = Frame.Spec;
	const FLinearColor& AxisColor = Opt.Colors.AxisColor;
	const FLinearColor& TitleColor = Opt.Colors.TitleColor;
	const FLinearColor& LabelColor = Opt.Colors.LabelColor;

	// ---- 十字准线
	Ctx.DrawDashedLine(FVector2D(LocalX, Plot.Min.Y), FVector2D(LocalX, Plot.Max.Y),
	                   4.0f, 4.0f, CrosshairColor, 1.0f);

	// ---- 每条系列上的最近点 + 提示行
	struct FTooltipRow
	{
		FLinearColor Color = FLinearColor::White;
		FString Name;
		FString Value;
		FSlateFontInfo NameFont;
		double NameWidth = 0.0;
		double ValueWidth = 0.0;
	};

	TArray<FTooltipRow> Rows;

	for (const FChartSeriesGeometry& Series : Frame.Series)
	{
		if (Series.LocalPoints.Num() == 0)
		{
			continue;
		}

		// 与外部准星共用同一套最近点算法
		const int32 BestIndex = ChartQuery::FindNearestPointIndex(Series.LocalPoints, LocalX);
		if (BestIndex == INDEX_NONE)
		{
			continue;
		}

		const FVector2D PointLocal = Series.LocalPoints[BestIndex];
		const FVector2D Value = Frame.Space.LocalToData(PointLocal);

		Ctx.DrawCircle(PointLocal, 3.5f, Series.Color, 2.0f, 12);

		// 名字用 DisplayName（留空回退 SeriesKey），过长则自动缩字号
		const FChartSeriesSpec* SeriesSpec = Frame.Spec.FindSeries(Series.SeriesKey);
		const FString Label = SeriesSpec ? SeriesSpec->GetLegendLabel() : Series.SeriesKey.ToString();

		FTooltipRow& Row = Rows.AddDefaulted_GetRef();
		Row.Color = Series.Color;
		Row.Name = Label;
		Row.Value = FormatValueWithStep(Value.Y, Layout.YAxis.TickSet.Step, Frame.Spec.AxisY.Format);
		Row.NameFont = FitFontToWidth(Layout.AxisLabelFont, Label, MaxTooltipNameWidth, Row.NameWidth);
		Row.ValueWidth = FChartPaintContext::MeasureText(Row.Value, Layout.AxisLabelFont).X;
	}

	if (Rows.Num() == 0)
	{
		return;
	}

	// ---- 表头：悬停位置的 X 值。外部准星直接显示传入值，避免像素往返丢精度
	const double HeaderValue = bExternal
		? Widget->GetCrosshairX()
		: Frame.Space.LocalToData(FVector2D(HoverLocal.X, HoverLocal.Y)).X;
	const FString Header = FormatValueWithStep(HeaderValue, Layout.XAxis.TickSet.Step, Frame.Spec.AxisX.Format);
	const FVector2D HeaderSize = FChartPaintContext::MeasureText(Header, Layout.AxisLabelFont);

	constexpr double Padding = 8.0;
	constexpr double SwatchSize = 10.0;
	constexpr double SwatchGap = 6.0;
	constexpr double NameValueGap = 10.0;

	double MaxNameWidth = 0.0;
	double MaxValueWidth = 0.0;
	double RowHeight = 0.0;

	for (const FTooltipRow& Row : Rows)
	{
		MaxNameWidth = FMath::Max(MaxNameWidth, Row.NameWidth);
		MaxValueWidth = FMath::Max(MaxValueWidth, Row.ValueWidth);
		RowHeight = FMath::Max(RowHeight, FChartPaintContext::MeasureText(TEXT("Ag"), Layout.AxisLabelFont).Y);
	}

	const double RowContentWidth = SwatchSize + SwatchGap + MaxNameWidth + NameValueGap + MaxValueWidth;
	const double ContentWidth = FMath::Max(HeaderSize.X, RowContentWidth);
	const double BoxWidth = ContentWidth + Padding * 2.0;
	const double BoxHeight = HeaderSize.Y + Padding * 2.0 + RowHeight * Rows.Num();

	// ---- 定位：优先放竖线右侧，越界翻到左侧，再不行贴边
	double BoxX = LocalX + 14.0;
	if (BoxX + BoxWidth > Plot.Max.X)
	{
		BoxX = LocalX - 14.0 - BoxWidth;
	}
	BoxX = FMath::Clamp(BoxX, Plot.Min.X, FMath::Max(Plot.Min.X, Plot.Max.X - BoxWidth));

	double BoxY;
	if (bExternal)
	{
		// 外部准星没有光标：信息框固定贴在绘图区顶部，跟随竖线横移
		BoxY = Plot.Min.Y + 8.0;
	}
	else
	{
		BoxY = HoverLocal.Y - BoxHeight * 0.5;
		BoxY = FMath::Clamp(BoxY, Plot.Min.Y, FMath::Max(Plot.Min.Y, Plot.Max.Y - BoxHeight));
	}

	const FVector2D BoxMin(BoxX, BoxY);

	Ctx.DrawRect(BoxMin, FVector2D(BoxWidth, BoxHeight), TooltipBackgroundColor);
	Ctx.DrawRectOutline(FBox2D(BoxMin, FVector2D(BoxMin.X + BoxWidth, BoxMin.Y + BoxHeight)), AxisColor, 1.0f);

	Ctx.DrawText(Header, FVector2D(BoxMin.X + Padding, BoxMin.Y + Padding), Layout.AxisLabelFont, TitleColor,
	             EChartTextHAlign::Left, EChartTextVAlign::Top);

	double RowY = BoxMin.Y + Padding + HeaderSize.Y;
	for (const FTooltipRow& Row : Rows)
	{
		Ctx.DrawRect(FVector2D(BoxMin.X + Padding, RowY + 3.0), FVector2D(SwatchSize, SwatchSize), Row.Color);

		// 名字左对齐（可能缩过字号），数值右对齐，方便纵向比对
		Ctx.DrawText(Row.Name, FVector2D(BoxMin.X + Padding + SwatchSize + SwatchGap, RowY),
		             Row.NameFont, LabelColor, EChartTextHAlign::Left, EChartTextVAlign::Top);

		Ctx.DrawText(Row.Value, FVector2D(BoxMin.X + BoxWidth - Padding, RowY),
		             Layout.AxisLabelFont, LabelColor, EChartTextHAlign::Right, EChartTextVAlign::Top);

		RowY += RowHeight;
	}
}
}

EChartLayer FHoverPainter::GetLayer() const { return EChartLayer::Tooltip; }

bool FHoverPainter::IsEnabled(const FChartPaintArgs& Args) const
{
	if (!Args.Frame.bValid)
	{
		return false;
	}

	if (Args.bHoverActive)
	{
		return true;
	}

	return Args.Widget != nullptr && Args.Widget->IsCrosshairActive();
}

void FHoverPainter::Paint(const FChartPaintArgs& Args) const { PaintHoverImpl(Args); }