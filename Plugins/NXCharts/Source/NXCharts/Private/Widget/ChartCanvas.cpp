#include "Widget/ChartCanvas.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"
#include "Layout/ChartTicks.h"
#include "Widget/NXChartWidget.h"

namespace
{
	constexpr float AxisLineThickness = 1.0f;
	constexpr double XLabelGap = 5.0;
	constexpr double YLabelGap = 7.0;
	constexpr double LegendMarkerSize = 10.0;
	constexpr double LegendMarkerGap = 5.0;

	/** 图例命中：矩形来自布局阶段，与绘制共用同一份结果 */
	FName FindLegendItemAt(const FChartFrame& Frame, const FVector2D& LocalPosition)
	{
		for (const FChartLegendItem& Entry : Frame.Layout.LegendItems)
		{
			if (LocalPosition.X >= Entry.Rect.Min.X && LocalPosition.X <= Entry.Rect.Max.X
				&& LocalPosition.Y >= Entry.Rect.Min.Y && LocalPosition.Y <= Entry.Rect.Max.Y)
			{
				return Entry.SeriesKey;
			}
		}
		return NAME_None;
	}

	const FLinearColor CrosshairColor(0.55f, 0.62f, 0.72f, 0.75f);
	const FLinearColor TooltipBackgroundColor(0.06f, 0.08f, 0.11f, 0.94f);

	/** 用轴刻度的步长决定小数位，保证 tooltip 与轴标签精度一致 */
	FString FormatValueWithStep(double Value, double Step, EChartTickFormat Format)
	{
		if (Step <= 0.0)
		{
			return FString::Printf(TEXT("%.2f"), Value);
		}
		return ChartTicks::FormatValue(Value, Step, Format);
	}
}

void SChartCanvas::Construct(const FArguments& InArgs, UNXChartWidget* InOwner)
{
	Owner = InOwner;
}

FVector2D SChartCanvas::ComputeDesiredSize(float) const
{
	return FVector2D(480.0f, 320.0f);
}

int32 SChartCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                            const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                            int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	UNXChartWidget* Widget = Owner.Get();
	if (!Widget)
	{
		return LayerId;
	}

	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	LastPaintSize = LocalSize;

	// 只读快照；必要时由 Widget 内部重建（绘制期不做任何映射计算）
	const FChartFrame& Frame = Widget->GetFrame(LocalSize);

	FChartPaintContext Ctx(OutDrawElements, AllottedGeometry, LayerId);

	// 背景（可选垂直渐变）
	if (Frame.Spec.Options.bGradientBackground)
	{
		Ctx.DrawVerticalGradient(FBox2D(FVector2D::ZeroVector, LocalSize),
		                         Frame.Spec.Options.BackgroundColor,
		                         Frame.Spec.Options.BackgroundColorBottom);
	}
	else
	{
		Ctx.DrawRect(FVector2D::ZeroVector, LocalSize, Frame.Spec.Options.BackgroundColor);
	}

	if (!Frame.bValid)
	{
		return Ctx.GetCurrentLayer();
	}

	PaintGridAndAxes(Ctx, Frame);
	PaintSeries(Ctx, Frame);
	PaintLegend(Ctx, Frame);
	PaintHover(Ctx, Frame);

	return Ctx.GetCurrentLayer();
}

void SChartCanvas::PaintGridAndAxes(FChartPaintContext& Ctx, const FChartFrame& Frame) const
{
	const FChartLayout& Layout = Frame.Layout;
	const FBox2D& Plot = Layout.PlotRect;

	// 颜色全部来自 Spec，画布不再持有任何硬编码配色
	const FChartOptionsSpec& Opt = Frame.Spec.Options;
	const FLinearColor& AxisColor = Opt.AxisColor;
	const FLinearColor& LabelColor = Opt.LabelColor;
	const FLinearColor& TitleColor = Opt.TitleColor;

	auto DrawGridSegment = [&Ctx, &Opt](const FVector2D& From, const FVector2D& To)
	{
		if (Opt.bDashedGrid)
		{
			Ctx.DrawDashedLine(From, To, Opt.GridDashLength, Opt.GridGapLength, Opt.GridColor, 1.0f);
		}
		else
		{
			Ctx.DrawLine(TArray<FVector2d>({ FVector2d(From), FVector2d(To) }), Opt.GridColor, 1.0f, false);
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
	if (!Frame.Spec.Options.Title.IsEmpty())
	{
		Ctx.DrawText(Frame.Spec.Options.Title.ToString(), Layout.TitleRect.Min, Layout.TitleFont, TitleColor,
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

void SChartCanvas::PaintSeries(FChartPaintContext& Ctx, const FChartFrame& Frame) const
{
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

void SChartCanvas::PaintLegend(FChartPaintContext& Ctx, const FChartFrame& Frame) const
{
	const FChartLayout& Layout = Frame.Layout;
	if (!Layout.bHasLegend || Layout.LegendItems.Num() == 0)
	{
		return;
	}

	const FLinearColor& BaseLabelColor = Frame.Spec.Options.LegendTextColor;

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

FReply SChartCanvas::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UNXChartWidget* Widget = Owner.Get();
	if (!Widget)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	// ---- 中键拖拽平移：以按下瞬间的空间为基准，避免累积误差
	if (bPanning)
	{
		const FVector2D StartData = PanStartSpace.LocalToData(PanStartLocal);
		const FVector2D CurrentData = PanStartSpace.LocalToData(LocalPosition);
		const FVector2D Delta = StartData - CurrentData; // 向右拖 → 数据域左移

		FChartDomain XDomain = PanStartSpace.GetXDomain();
		FChartDomain YDomain = PanStartSpace.GetYDomain();
		XDomain.Min += Delta.X;
		XDomain.Max += Delta.X;
		YDomain.Min += Delta.Y;
		YDomain.Max += Delta.Y;

		FChartDomainOverride Override = Widget->GetDomainOverride();
		Override.bOverrideX = true;
		Override.XRange = FVector2D(XDomain.Min, XDomain.Max);
		Override.bOverrideY = true;
		Override.YRange = FVector2D(YDomain.Min, YDomain.Max);
		Widget->SetDomainOverrideRaw(Override);

		return FReply::Handled();
	}

	const FChartFrame& Frame = Widget->GetFrame(MyGeometry.GetLocalSize());
	const FBox2D& Plot = Frame.Layout.PlotRect;

	const bool bInside = Frame.bValid
		&& LocalPosition.X >= Plot.Min.X && LocalPosition.X <= Plot.Max.X
		&& LocalPosition.Y >= Plot.Min.Y && LocalPosition.Y <= Plot.Max.Y;

	if (!bInside)
	{
		if (bHoverActive)
		{
			bHoverActive = false;
			Invalidate(EInvalidateWidget::Paint);
		}
		return FReply::Unhandled();
	}

	bHoverActive = true;
	HoverLocal = LocalPosition;

	// 找最近的数据点，只在目标变化时向蓝图广播，避免每次鼠标移动都刷事件
	FName NewKey = NAME_None;
	int32 NewIndex = INDEX_NONE;
	FVector2D NewValue = FVector2D::ZeroVector;
	double BestDistance = TNumericLimits<double>::Max();

	for (const FChartSeriesGeometry& Series : Frame.Series)
	{
		for (int32 Index = 0; Index < Series.LocalPoints.Num(); ++Index)
		{
			const double Distance = FMath::Abs(Series.LocalPoints[Index].X - LocalPosition.X);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				NewKey = Series.SeriesKey;
				NewIndex = Index;
				NewValue = Frame.Space.LocalToData(Series.LocalPoints[Index]);
			}
		}
	}

	if (NewKey != LastHoverSeriesKey || NewIndex != LastHoverPointIndex)
	{
		LastHoverSeriesKey = NewKey;
		LastHoverPointIndex = NewIndex;
		Widget->BroadcastHover(NewKey, NewIndex, NewValue);
	}

	Invalidate(EInvalidateWidget::Paint);
	return FReply::Handled();
}

void SChartCanvas::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (bHoverActive)
	{
		bHoverActive = false;
		LastHoverSeriesKey = NAME_None;
		LastHoverPointIndex = INDEX_NONE;
		Invalidate(EInvalidateWidget::Paint);
	}

	SLeafWidget::OnMouseLeave(MouseEvent);
}

FReply SChartCanvas::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UNXChartWidget* Widget = Owner.Get();
	if (!Widget)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	// 左键：点图例切换该系列的显隐
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FChartFrame& CurrentFrame = Widget->GetFrame(MyGeometry.GetLocalSize());
		const FName LegendKey = FindLegendItemAt(CurrentFrame, LocalPosition);
		if (!LegendKey.IsNone())
		{
			if (const FChartSeriesSpec* SeriesSpec = CurrentFrame.Spec.FindSeries(LegendKey))
			{
				Widget->SetSeriesVisible(LegendKey, !SeriesSpec->bVisible);
				return FReply::Handled();
			}
		}
		return FReply::Unhandled();
	}

	// 中键：拖拽平移
	if (MouseEvent.GetEffectingButton() != EKeys::MiddleMouseButton)
	{
		return FReply::Unhandled();
	}

	const FChartFrame& Frame = Widget->GetFrame(MyGeometry.GetLocalSize());
	if (!Frame.bValid || !Frame.Layout.PlotRect.IsInside(LocalPosition))
	{
		return FReply::Unhandled();
	}

	bPanning = true;
	PanStartLocal = LocalPosition;
	PanStartSpace = Frame.Space;

	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SChartCanvas::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::MiddleMouseButton || !bPanning)
	{
		return FReply::Unhandled();
	}

	bPanning = false;
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SChartCanvas::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UNXChartWidget* Widget = Owner.Get();
	if (!Widget)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FChartFrame& Frame = Widget->GetFrame(MyGeometry.GetLocalSize());
	if (!Frame.bValid || !Frame.Layout.PlotRect.IsInside(LocalPosition))
	{
		return FReply::Unhandled();
	}

	const double WheelDelta = MouseEvent.GetWheelDelta();
	if (FMath::IsNearlyZero(WheelDelta))
	{
		return FReply::Unhandled();
	}

	// 向上滚（delta > 0）= 放大
	const double Factor = FMath::Pow(1.3, -WheelDelta);

	// 以光标所在的数据值为锚点缩放：锚点在屏幕上保持不动
	const FVector2D Anchor = Frame.Space.LocalToData(LocalPosition);

	auto ZoomDomain = [](const FChartDomain& Domain, double AnchorValue, double InFactor)
	{
		const double NewSpan = Domain.Size() * InFactor;
		if (NewSpan < 1e-9 || NewSpan > 1e12)
		{
			return Domain; // 到达极限，保持不变
		}

		FChartDomain Result;
		Result.bValid = true;
		Result.Min = AnchorValue + (Domain.Min - AnchorValue) * InFactor;
		Result.Max = AnchorValue + (Domain.Max - AnchorValue) * InFactor;
		return Result;
	};

	const FChartDomain XDomain = ZoomDomain(Frame.Space.GetXDomain(), Anchor.X, Factor);
	const FChartDomain YDomain = ZoomDomain(Frame.Space.GetYDomain(), Anchor.Y, Factor);

	FChartDomainOverride Override = Widget->GetDomainOverride();
	Override.bOverrideX = true;
	Override.XRange = FVector2D(XDomain.Min, XDomain.Max);
	Override.bOverrideY = true;
	Override.YRange = FVector2D(YDomain.Min, YDomain.Max);
	Widget->SetDomainOverrideRaw(Override);

	return FReply::Handled();
}

void SChartCanvas::PaintHover(FChartPaintContext& Ctx, const FChartFrame& Frame) const
{
	if (!bHoverActive || !Frame.bValid)
	{
		return;
	}

	const FChartLayout& Layout = Frame.Layout;
	const FBox2D& Plot = Layout.PlotRect;

	const FChartOptionsSpec& Opt = Frame.Spec.Options;
	const FLinearColor& AxisColor = Opt.AxisColor;
	const FLinearColor& TitleColor = Opt.TitleColor;
	const FLinearColor& LabelColor = Opt.LabelColor;

	// ---- 十字准线
	Ctx.DrawDashedLine(FVector2D(HoverLocal.X, Plot.Min.Y), FVector2D(HoverLocal.X, Plot.Max.Y),
	                   4.0f, 4.0f, CrosshairColor, 1.0f);

	// ---- 每条系列上的最近点 + 提示行
	TArray<FLinearColor> RowColors;
	TArray<FString> RowTexts;

	double MaxRowWidth = 0.0;
	double RowHeight = 0.0;

	for (const FChartSeriesGeometry& Series : Frame.Series)
	{
		if (Series.LocalPoints.Num() == 0)
		{
			continue;
		}

		int32 BestIndex = INDEX_NONE;
		double BestDistance = TNumericLimits<double>::Max();
		for (int32 Index = 0; Index < Series.LocalPoints.Num(); ++Index)
		{
			const double Distance = FMath::Abs(Series.LocalPoints[Index].X - HoverLocal.X);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestIndex = Index;
			}
		}

		if (BestIndex == INDEX_NONE)
		{
			continue;
		}

		const FVector2D PointLocal = Series.LocalPoints[BestIndex];
		const FVector2D Value = Frame.Space.LocalToData(PointLocal);

		Ctx.DrawCircle(PointLocal, 3.5f, Series.Color, 2.0f, 12);

		const FString Text = FString::Printf(TEXT("%s   %s"), *Series.SeriesKey.ToString(),
			*FormatValueWithStep(Value.Y, Layout.YAxis.TickSet.Step, Frame.Spec.AxisY.Format));

		MaxRowWidth = FMath::Max(MaxRowWidth, FChartPaintContext::MeasureText(Text, Layout.AxisLabelFont).X);
		RowHeight = FMath::Max(RowHeight, FChartPaintContext::MeasureText(TEXT("Ag"), Layout.AxisLabelFont).Y);

		RowColors.Add(Series.Color);
		RowTexts.Add(Text);
	}

	if (RowTexts.Num() == 0)
	{
		return;
	}

	// ---- 表头：悬停位置的 X 值
	const FVector2D HoverData = Frame.Space.LocalToData(FVector2D(HoverLocal.X, HoverLocal.Y));
	const FString Header = FormatValueWithStep(HoverData.X, Layout.XAxis.TickSet.Step, Frame.Spec.AxisX.Format);
	const FVector2D HeaderSize = FChartPaintContext::MeasureText(Header, Layout.AxisLabelFont);
	MaxRowWidth = FMath::Max(MaxRowWidth, HeaderSize.X);

	constexpr double Padding = 8.0;
	constexpr double SwatchSize = 10.0;
	constexpr double SwatchGap = 6.0;

	const double BoxWidth = MaxRowWidth + SwatchSize + SwatchGap + Padding * 2.0;
	const double BoxHeight = HeaderSize.Y + Padding * 2.0 + RowHeight * RowTexts.Num();

	// 优先放光标右侧，越界则翻到左侧，再不行就贴边
	double BoxX = HoverLocal.X + 14.0;
	if (BoxX + BoxWidth > Plot.Max.X)
	{
		BoxX = HoverLocal.X - 14.0 - BoxWidth;
	}
	BoxX = FMath::Clamp(BoxX, Plot.Min.X, FMath::Max(Plot.Min.X, Plot.Max.X - BoxWidth));

	double BoxY = HoverLocal.Y - BoxHeight * 0.5;
	BoxY = FMath::Clamp(BoxY, Plot.Min.Y, FMath::Max(Plot.Min.Y, Plot.Max.Y - BoxHeight));

	const FVector2D BoxMin(BoxX, BoxY);

	Ctx.DrawRect(BoxMin, FVector2D(BoxWidth, BoxHeight), TooltipBackgroundColor);
	Ctx.DrawRectOutline(FBox2D(BoxMin, FVector2D(BoxMin.X + BoxWidth, BoxMin.Y + BoxHeight)), AxisColor, 1.0f);

	Ctx.DrawText(Header, FVector2D(BoxMin.X + Padding, BoxMin.Y + Padding), Layout.AxisLabelFont, TitleColor,
	             EChartTextHAlign::Left, EChartTextVAlign::Top);

	double RowY = BoxMin.Y + Padding + HeaderSize.Y;
	for (int32 RowIndex = 0; RowIndex < RowTexts.Num(); ++RowIndex)
	{
		Ctx.DrawRect(FVector2D(BoxMin.X + Padding, RowY + 3.0), FVector2D(SwatchSize, SwatchSize), RowColors[RowIndex]);
		Ctx.DrawText(RowTexts[RowIndex], FVector2D(BoxMin.X + Padding + SwatchSize + SwatchGap, RowY),
		             Layout.AxisLabelFont, LabelColor, EChartTextHAlign::Left, EChartTextVAlign::Top);
		RowY += RowHeight;
	}
}
