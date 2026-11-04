// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NXChartWidget.h"

#include "Frame/ChartFrameBuilder.h"
#include "Widget/ChartCanvas.h"

// ---------------------------------------------------------------------------- UWidget

TSharedRef<SWidget> UNXChartWidget::RebuildWidget()
{
	ChartCanvas = SNew(SChartCanvas, this);
	return ChartCanvas.ToSharedRef();
}

void UNXChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	ChartCanvas.Reset();
}

void UNXChartWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	MarkSpecDirty();
}

#if WITH_EDITOR
void UNXChartWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	MarkSpecDirty();
}

const FText UNXChartWidget::GetPaletteCategory()
{
	return FText::FromString(TEXT("NXCharts"));
}
#endif

// ---------------------------------------------------------------------------- 失效

void UNXChartWidget::MarkSpecDirty()
{
	++SpecRevision;
	BuiltSpecRevision = -1;
	InvalidateChart();
}

void UNXChartWidget::InvalidateChart()
{
	if (ChartCanvas.IsValid())
	{
		ChartCanvas->Invalidate(EInvalidateWidget::Paint);
	}
}

void UNXChartWidget::RefreshChart()
{
	MarkSpecDirty();
}

// ---------------------------------------------------------------------------- 运行时配置

void UNXChartWidget::SetChartSpec(const FChartSpec& InSpec)
{
	Spec = InSpec;
	MarkSpecDirty();
}

void UNXChartWidget::SetAxisSpec(EChartAxis Axis, const FChartAxisSpec& AxisSpec)
{
	if (Axis == EChartAxis::X)
	{
		Spec.AxisX = AxisSpec;
	}
	else
	{
		Spec.AxisY = AxisSpec;
	}
	MarkSpecDirty();
}

void UNXChartWidget::AddOrUpdateSeries(const FChartSeriesSpec& SeriesSpec)
{
	if (!SeriesSpec.IsValid())
	{
		return;
	}

	if (FChartSeriesSpec* Existing = Spec.FindSeriesMutable(SeriesSpec.SeriesKey))
	{
		*Existing = SeriesSpec;
	}
	else
	{
		Spec.Series.Add(SeriesSpec);
	}

	MarkSpecDirty();
}

void UNXChartWidget::RemoveSeries(FName SeriesKey)
{
	const int32 Index = Spec.IndexOfSeries(SeriesKey);
	if (Index == INDEX_NONE)
	{
		return;
	}

	Spec.Series.RemoveAt(Index);
	DataPool.Remove(SeriesKey);
	MarkSpecDirty();
}

void UNXChartWidget::SetSeriesVisible(FName SeriesKey, bool bVisible)
{
	FChartSeriesSpec* Existing = Spec.FindSeriesMutable(SeriesKey);
	if (!Existing || Existing->bVisible == bVisible)
	{
		return;
	}

	Existing->bVisible = bVisible;
	MarkSpecDirty();
}

void UNXChartWidget::SetSeriesColor(FName SeriesKey, FLinearColor Color)
{
	FChartSeriesSpec* Existing = Spec.FindSeriesMutable(SeriesKey);
	if (!Existing)
	{
		return;
	}

	Existing->Style.Color = Color;
	MarkSpecDirty();
}

void UNXChartWidget::AddSeries(FName SeriesKey, EChartSeriesKind Kind, FLinearColor Color,
                             const TArray<FVector2D>& Points, FString DisplayName)
{
	if (SeriesKey.IsNone())
	{
		return;
	}

	// 已有同 Key 时在原有配置上更新，保留填充/发光等未在此处暴露的样式
	FChartSeriesSpec SeriesSpec;
	if (const FChartSeriesSpec* Existing = Spec.FindSeries(SeriesKey))
	{
		SeriesSpec = *Existing;
	}
	else
	{
		SeriesSpec.SeriesKey = SeriesKey;
	}

	SeriesSpec.Kind = Kind;
	SeriesSpec.DisplayName = FText::FromString(DisplayName);
	SeriesSpec.Style.Color = Color;
	SeriesSpec.bVisible = true;

	AddOrUpdateSeries(SeriesSpec);

	if (Points.Num() > 0)
	{
		SetSeriesPoints(SeriesKey, Points);
	}
}

void UNXChartWidget::SetSeriesStyle(FName SeriesKey, const FChartSeriesStyle& Style)
{
	FChartSeriesSpec* Existing = Spec.FindSeriesMutable(SeriesKey);
	if (!Existing)
	{
		return;
	}

	Existing->Style = Style;
	MarkSpecDirty();
}

void UNXChartWidget::SetSeriesCurve(FName SeriesKey, EChartCurveType Curve)
{
	FChartSeriesSpec* Existing = Spec.FindSeriesMutable(SeriesKey);
	if (!Existing || Existing->Style.Curve == Curve)
	{
		return;
	}

	Existing->Style.Curve = Curve;
	MarkSpecDirty();
}

// ---------------------------------------------------------------------------- 运行时数据

void UNXChartWidget::SetSeriesPoints(FName SeriesKey, const TArray<FVector2D>& Points)
{
	if (SeriesKey.IsNone())
	{
		return;
	}

	FChartSeriesBuffer& Buffer = DataPool.FindOrAdd(SeriesKey);
	Buffer.Capacity = SeriesCapacity;
	Buffer.SetPoints(Points);

	InvalidateChart();
}

void UNXChartWidget::AddSeriesPoint(FName SeriesKey, FVector2D Point)
{
	if (SeriesKey.IsNone())
	{
		return;
	}

	FChartSeriesBuffer& Buffer = DataPool.FindOrAdd(SeriesKey);
	Buffer.Capacity = SeriesCapacity;
	Buffer.AddPoint(Point);

	InvalidateChart();
}

void UNXChartWidget::AddSeriesPoints(FName SeriesKey, TArrayView<const FVector2d> Points)
{
	if (SeriesKey.IsNone() || Points.Num() == 0)
	{
		return;
	}

	FChartSeriesBuffer& Buffer = DataPool.FindOrAdd(SeriesKey);
	Buffer.Capacity = SeriesCapacity;
	Buffer.AddPoints(Points);

	InvalidateChart();
}

void UNXChartWidget::ClearSeries(FName SeriesKey)
{
	if (FChartSeriesBuffer* Buffer = DataPool.FindMutable(SeriesKey))
	{
		Buffer->Reset();
		InvalidateChart();
	}
}

// ---------------------------------------------------------------------------- 查询

TArray<FName> UNXChartWidget::GetSeriesKeys() const
{
	TArray<FName> Keys;
	Keys.Reserve(Spec.Series.Num());
	for (const FChartSeriesSpec& Item : Spec.Series)
	{
		Keys.Add(Item.SeriesKey);
	}
	return Keys;
}

void UNXChartWidget::BroadcastHover(FName SeriesKey, int32 PointIndex, FVector2D DataValue)
{
	OnChartHovered.Broadcast(SeriesKey, PointIndex, DataValue);
}

// ---------------------------------------------------------------------------- 运行时域覆盖

void UNXChartWidget::SetDomainOverride(FVector2D XRange, FVector2D YRange)
{
	DomainOverride.bOverrideX = (XRange.X != XRange.Y);
	DomainOverride.XRange = XRange;
	DomainOverride.bOverrideY = (YRange.X != YRange.Y);
	DomainOverride.YRange = YRange;

	MarkSpecDirty();
}

void UNXChartWidget::ClearDomainOverride()
{
	DomainOverride.Clear();
	MarkSpecDirty();
}

bool UNXChartWidget::HasDomainOverride() const
{
	return DomainOverride.HasAny();
}

void UNXChartWidget::SetDomainOverrideRaw(const FChartDomainOverride& InOverride)
{
	DomainOverride = InOverride;
	MarkSpecDirty();
}

// ---------------------------------------------------------------------------- 帧

const FChartFrame& UNXChartWidget::GetFrame(const FVector2D& WidgetSize)
{
	const int64 DataRevision = DataPool.GetRevisionSum();
	const bool bSpecChanged = (BuiltSpecRevision != SpecRevision);
	const bool bSizeChanged = !BuiltSize.Equals(WidgetSize, 0.5);

	if (bSpecChanged || bSizeChanged)
	{
		// 配置或尺寸变化：域、刻度、布局、全部几何都要重算
		FChartFrameBuilder::Build(Spec, DataPool, DomainOverride, WidgetSize, CachedFrame);
	}
	else if (BuiltDataRevisionSum != DataRevision)
	{
		// 只有数据变了：域不变时只重建变化的那几条系列几何（跳过布局与文字测量）
		FChartFrameBuilder::ReconcileData(CachedFrame, Spec, DataPool, DomainOverride, WidgetSize);
	}
	else
	{
		return CachedFrame;
	}

	BuiltSpecRevision = SpecRevision;
	BuiltDataRevisionSum = DataRevision;
	BuiltSize = WidgetSize;
	return CachedFrame;
}
