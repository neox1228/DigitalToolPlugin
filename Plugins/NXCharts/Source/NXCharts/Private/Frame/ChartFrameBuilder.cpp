// Fill out your copyright notice in the Description page of Project Settings.


#include "Frame/ChartFrameBuilder.h"

#include "Layout/ChartLayout.h"
#include "Frame/ChartCurve.h"
#include "Frame/ChartDecimator.h"

namespace
{
	/** 曲线细分后的顶点上限，防御异常数据把 Slate 拖死 */
	constexpr int32 MaxCurvePoints = 20000;

	/** 把域夹进轴的硬限制区间；数据完全落在区间外时退回展示整个区间 */
	FChartDomain ClampToHardLimits(FChartDomain Domain, const FChartAxisSpec& AxisSpec)
	{
		if (!AxisSpec.bUseHardLimits)
		{
			return Domain;
		}

		const double Lower = FMath::Min(AxisSpec.HardLimits.X, AxisSpec.HardLimits.Y);
		const double Upper = FMath::Max(AxisSpec.HardLimits.X, AxisSpec.HardLimits.Y);
		if (Upper - Lower <= KINDA_SMALL_NUMBER)
		{
			return Domain;
		}

		Domain.Min = FMath::Max(Domain.Min, Lower);
		Domain.Max = FMath::Min(Domain.Max, Upper);

		if (Domain.Max - Domain.Min <= KINDA_SMALL_NUMBER)
		{
			Domain.Min = Lower;
			Domain.Max = Upper;
		}

		Domain.bValid = true;
		return Domain;
	}

	bool DomainEqual(const FChartDomain& A, const FChartDomain& B)
	{
		if (A.bValid != B.bValid)
		{
			return false;
		}

		// 相对误差比较：自动域是按数据算出来的，浮点累加会出现末位差异
		const double Scale = FMath::Max(1.0, FMath::Max(FMath::Abs(A.Min), FMath::Abs(A.Max)));
		const double Epsilon = Scale * 1e-9;
		return FMath::Abs(A.Min - B.Min) <= Epsilon && FMath::Abs(A.Max - B.Max) <= Epsilon;
	}

	FChartSeriesGeometry BuildSeriesGeometry(const FChartSeriesSpec& SeriesSpec,
	                                         const FChartSeriesBuffer& Buffer,
	                                         const FChartCartesianSpace& Space)
	{
		FChartSeriesGeometry Geometry;
		Geometry.SeriesKey = SeriesSpec.SeriesKey;
		Geometry.Kind = SeriesSpec.Kind;
		Geometry.Color = SeriesSpec.Style.Color;
		Geometry.LineThickness = SeriesSpec.Style.LineThickness;
		Geometry.bAntialias = SeriesSpec.Style.bAntialias;
		Geometry.bShowMarkers = SeriesSpec.Style.bShowMarkers;
		Geometry.MarkerSize = SeriesSpec.Style.MarkerSize;
		Geometry.bFillArea = SeriesSpec.Style.bFillArea;
		Geometry.AreaFillOpacity = SeriesSpec.Style.AreaFillOpacity;
		Geometry.bGlow = SeriesSpec.Style.bGlow;
		Geometry.GlowThickness = SeriesSpec.Style.GlowThickness;
		Geometry.DataRevision = Buffer.Revision;

		const TArray<FVector2d>& Points = Buffer.GetPoints();

		// 按像素列抽稀：顶点数上界从 O(数据量) 压到 O(绘图区宽度)
		const FChartDomain& XDomain = Space.GetXDomain();
		const double PlotWidth = Space.GetPlotRect().GetSize().X;
		const int32 MaxPoints = FMath::Max(64, FMath::RoundToInt(PlotWidth) * 2);

		TArray<int32> Indices;
		ChartDecimate::MinMaxByColumn(Points, XDomain.Min, XDomain.Max, PlotWidth, MaxPoints, Indices);

		TArray<FVector2d> RawPoints;
		RawPoints.Reserve(Indices.Num());
		for (int32 Index : Indices)
		{
			RawPoints.Add(Space.DataToLocal(Points[Index]));
		}

		// 曲线化必须在抽稀之后：先压点数再细分，否则万点样条会爆
		switch (SeriesSpec.Style.Curve)
		{
		case EChartCurveType::Smooth:
			ChartCurve::SmoothMonotone(RawPoints, MaxCurvePoints, Geometry.LocalPoints);
			break;

		case EChartCurveType::Step:
			ChartCurve::Step(RawPoints, Geometry.LocalPoints);
			break;

		default:
			Geometry.LocalPoints = MoveTemp(RawPoints);
			break;
		}

		return Geometry;
	}

}

FChartDomain FChartFrameBuilder::ResolveDomain(EChartAxis Axis, const FChartAxisSpec& AxisSpec,
                                               const FChartSpec& Spec, const FChartSeriesDataPool& Data,
                                               const FChartDomainOverride& Override)
{
	const bool bXAxis = (Axis == EChartAxis::X);

	// 1) 运行时覆盖（缩放 / 平移）优先级最高
	if (bXAxis && Override.bOverrideX)
	{
		return ClampToHardLimits(FChartDomain::FromRange(Override.XRange.X, Override.XRange.Y), AxisSpec);
	}
	if (!bXAxis && Override.bOverrideY)
	{
		return ClampToHardLimits(FChartDomain::FromRange(Override.YRange.X, Override.YRange.Y), AxisSpec);
	}

	// 2) 配置里写死的范围
	if (AxisSpec.DomainMode == EChartDomainMode::Manual)
	{
		return ClampToHardLimits(FChartDomain::FromRange(AxisSpec.ManualDomain.X, AxisSpec.ManualDomain.Y), AxisSpec);
	}

	// 3) 自动推导
	FChartDomain Domain;
	for (const FChartSeriesSpec& SeriesSpec : Spec.Series)
	{
		if (!SeriesSpec.bVisible)
		{
			continue;
		}

		// X 轴纳入所有可见系列；Y 轴只纳入绑定到本轴的系列（为双 Y 轴预留）
		if (!bXAxis && SeriesSpec.ValueAxis != Axis)
		{
			continue;
		}

		const FChartSeriesBuffer* Buffer = Data.Find(SeriesSpec.SeriesKey);
		if (!Buffer)
		{
			continue;
		}

		for (const FVector2d& Point : Buffer->GetPoints())
		{
			Domain.Include(bXAxis ? Point.X : Point.Y);
		}
	}

	if (!Domain.IsValid())
	{
		// 没有数据时也给一个可画的默认域，避免出现空白控件
		Domain = FChartDomain::FromRange(0.0, 1.0);
	}

	Domain.Normalize();

	// Y 轴上下留 5% 余量，曲线不会贴死绘图区边界；X 轴不留，线条从边缘开始更符合预期
	if (!bXAxis)
	{
		Domain.Pad(0.05);
	}

	return ClampToHardLimits(Domain, AxisSpec);
}

bool FChartFrameBuilder::Build(const FChartSpec& Spec, const FChartSeriesDataPool& Data,
                               const FChartDomainOverride& Override, const FVector2D& WidgetSize,
                               FChartFrame& OutFrame, const FChartMeasureFunc& Measure)
{
	OutFrame.Reset();
	OutFrame.Spec = Spec;

	const FChartDomain XDomain = ResolveDomain(EChartAxis::X, Spec.AxisX, Spec, Data, Override);
	const FChartDomain YDomain = ResolveDomain(EChartAxis::Y, Spec.AxisY, Spec, Data, Override);

	OutFrame.Layout = FChartLayoutBuilder::Build(Spec, XDomain, YDomain, WidgetSize, Measure);
	if (!OutFrame.Layout.bValid)
	{
		return false;
	}

	OutFrame.Space.Setup(XDomain, YDomain, OutFrame.Layout.PlotRect);

	OutFrame.Series.Reserve(Spec.Series.Num());
	for (const FChartSeriesSpec& SeriesSpec : Spec.Series)
	{
		if (!SeriesSpec.bVisible)
		{
			continue;
		}

		const FChartSeriesBuffer* Buffer = Data.Find(SeriesSpec.SeriesKey);
		if (!Buffer || Buffer->Num() == 0)
		{
			continue;
		}

		OutFrame.Series.Emplace(BuildSeriesGeometry(SeriesSpec, *Buffer, OutFrame.Space));
	}

	OutFrame.DataRevisionSum = Data.GetRevisionSum();
	OutFrame.BuildSize = WidgetSize;
	OutFrame.bValid = true;
	return true;
}

bool FChartFrameBuilder::ReconcileData(FChartFrame& InOutFrame, const FChartSpec& Spec,
                                       const FChartSeriesDataPool& Data, const FChartDomainOverride& Override,
                                       const FVector2D& WidgetSize, const FChartMeasureFunc& Measure)
{
	// 没有可用帧时只能整帧重建
	if (!InOutFrame.bValid)
	{
		return Build(Spec, Data, Override, WidgetSize, InOutFrame, Measure);
	}

	const FChartDomain NewXDomain = ResolveDomain(EChartAxis::X, Spec.AxisX, Spec, Data, Override);
	const FChartDomain NewYDomain = ResolveDomain(EChartAxis::Y, Spec.AxisY, Spec, Data, Override);

	// 自动范围会随新数据变化，域一变刻度、标签、绘图区都会变，只能整帧重算
	const bool bDomainChanged = !DomainEqual(NewXDomain, InOutFrame.Space.GetXDomain())
		|| !DomainEqual(NewYDomain, InOutFrame.Space.GetYDomain());
	if (bDomainChanged)
	{
		return Build(Spec, Data, Override, WidgetSize, InOutFrame, Measure);
	}

	// ---- 域没变：只重建数据版本变化了的系列 ----
	TArray<FChartSeriesGeometry> Previous = MoveTemp(InOutFrame.Series);

	TMap<FName, int32> PreviousIndex;
	PreviousIndex.Reserve(Previous.Num());
	for (int32 Index = 0; Index < Previous.Num(); ++Index)
	{
		PreviousIndex.Add(Previous[Index].SeriesKey, Index);
	}

	InOutFrame.Series.Reset();
	InOutFrame.Series.Reserve(Previous.Num() + 1);

	for (const FChartSeriesSpec& SeriesSpec : Spec.Series)
	{
		if (!SeriesSpec.bVisible)
		{
			continue;
		}

		const FChartSeriesBuffer* Buffer = Data.Find(SeriesSpec.SeriesKey);
		if (!Buffer || Buffer->Num() == 0)
		{
			continue;
		}

		const int32* FoundIndex = PreviousIndex.Find(SeriesSpec.SeriesKey);
		if (FoundIndex && Previous[*FoundIndex].DataRevision == Buffer->Revision)
		{
			// 数据没变，整块几何（含顶点数组）原样搬过来，不做任何重建
			InOutFrame.Series.Emplace(MoveTemp(Previous[*FoundIndex]));
			continue;
		}

		InOutFrame.Series.Emplace(BuildSeriesGeometry(SeriesSpec, *Buffer, InOutFrame.Space));
	}

	InOutFrame.DataRevisionSum = Data.GetRevisionSum();
	InOutFrame.BuildSize = WidgetSize;
	return true;
}
