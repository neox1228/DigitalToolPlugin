#include "Widget/ChartCanvas.h"

#include "Core/ChartPaintContext.h"
#include "Frame/ChartFrame.h"
#include "Layout/ChartLayout.h"
#include "Layout/ChartTicks.h"
#include "Widget/NXChartWidget.h"
#include "Widget/Painters/AllPainters.h"
#include "Widget/Zoom/CartesianZoom.h"


namespace
{
	/**
	 * 把缩放结果夹进轴的取值范围限制。
	 * @param CurrentDomain 当前正在显示的域（判定基准，不能传缩放结果）
	 * @return false 表示夹紧后与当前域相同（已顶死在边界），这次缩放应被忽略
	 */
	bool ApplyHardLimits(const FChartAxisSpec& AxisSpec, const FChartDomain& CurrentDomain, FChartDomain& Domain)
	{
		if (!AxisSpec.bUseHardLimits)
		{
			return true;
		}

		const double Lower = FMath::Min(AxisSpec.HardLimits.X, AxisSpec.HardLimits.Y);
		const double Upper = FMath::Max(AxisSpec.HardLimits.X, AxisSpec.HardLimits.Y);
		return ChartDomainUtil::ClampDomainToRange(Domain, Lower, Upper, CurrentDomain);
	}

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
}


void SChartCanvas::Construct(const FArguments& InArgs, UNXChartWidget* InOwner)
{
	Owner = InOwner;
	SetCanTick(true);

	// 绘制组件注册表：加一段新绘制 = 在这里 Add 一行
	Painters.Add(MakeUnique<FBackgroundPainter>());
	Painters.Add(MakeUnique<FGridAndAxesPainter>());
	Painters.Add(MakeUnique<FAnnotationPainter>());
	Painters.Add(MakeUnique<FSeriesPainter>());
	Painters.Add(MakeUnique<FLegendPainter>());
	Painters.Add(MakeUnique<FHoverPainter>());

	Zoom = MakeUnique<FCartesianZoom>();

	Painters.StableSort([](const TUniquePtr<FChartPainter>& A, const TUniquePtr<FChartPainter>& B)
	{
		return A->GetLayer() < B->GetLayer();
	});
}

void SChartCanvas::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// 每 0.5 秒看一次配置文件有没有被改
	if (UNXChartWidget* OwnerWidget = Owner.Get(); OwnerWidget && !OwnerWidget->bAutoReloadSpec)
	{
		return;
	}

	SpecCheckAccumulator += InDeltaTime;
	if (SpecCheckAccumulator < 0.5f)
	{
		return;
	}
	SpecCheckAccumulator = 0.0f;

	if (UNXChartWidget* Widget = Owner.Get())
	{
		Widget->ReloadSpecIfChanged();
	}
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

	FChartPaintArgs PaintArgs{ Ctx, Frame, Widget, LocalSize, bHoverActive, HoverLocal };

	// 背景先画（帧无效时也要有底色）
	for (const TUniquePtr<FChartPainter>& Painter : Painters)
	{
		if (Painter && Painter->GetLayer() == EChartLayer::Background && Painter->IsEnabled(PaintArgs))
		{
			Painter->Paint(PaintArgs);
		}
	}

	if (!Frame.bValid)
	{
		return Ctx.GetCurrentLayer();
	}

	// 其余组件按层序遍历
	for (const TUniquePtr<FChartPainter>& Painter : Painters)
	{
		if (Painter && Painter->GetLayer() != EChartLayer::Background && Painter->IsEnabled(PaintArgs))
		{
			Painter->Paint(PaintArgs);
		}
	}

	return Ctx.GetCurrentLayer();
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

	// 左键：双击绘图区复位视图；单击图例切换显隐
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FChartFrame& CurrentFrame = Widget->GetFrame(MyGeometry.GetLocalSize());

		// 双击判定：350ms 内、8 像素以内
		const double Now = FPlatformTime::Seconds();
		const bool bDoubleClick = (Now - LastClickTime) < 0.35
			&& FVector2D::Distance(LocalPosition, LastClickPosition) < 8.0;
		LastClickTime = Now;
		LastClickPosition = LocalPosition;

		if (bDoubleClick && CurrentFrame.bValid && CurrentFrame.Layout.PlotRect.IsInside(LocalPosition))
		{
			Widget->ClearDomainOverride();
			return FReply::Handled();
		}

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

	if (!Zoom)
	{
		return FReply::Unhandled();
	}

	// 画布只负责转发，具体怎么缩放由策略决定
	FChartZoomArgs Args;
	Args.LocalPosition = LocalPosition;
	Args.WheelDelta = MouseEvent.GetWheelDelta();
	Args.Frame = &Frame;
	Args.Override = Widget->GetDomainOverride();

	if (Zoom->HandleWheel(Args))
	{
		Widget->SetDomainOverrideRaw(Args.Override);
	}

	return FReply::Handled();
}

