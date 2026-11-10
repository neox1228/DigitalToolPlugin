// Fill out your copyright notice in the Description page of Project Settings.


#include "Layout/ChartSpace.h"

// ---------------------------------------------------------------------------- FChartDomain

void FChartDomain::Reset()
{
	Min = 0.0;
	Max = 1.0;
	bValid = false;
}

void FChartDomain::Include(double Value)
{
	if (!FMath::IsFinite(Value))
	{
		return;
	}

	if (!bValid)
	{
		Min = Value;
		Max = Value;
		bValid = true;
		return;
	}

	Min = FMath::Min(Min, Value);
	Max = FMath::Max(Max, Value);
}

void FChartDomain::Include(const FChartDomain& Other)
{
	if (!Other.bValid)
	{
		return;
	}
	Include(Other.Min);
	Include(Other.Max);
}

bool FChartDomain::IsValid() const
{
	return bValid && Max > Min;
}

double FChartDomain::Size() const
{
	return Max - Min;
}

void FChartDomain::Normalize(double MinSpanRatio)
{
	if (!bValid)
	{
		Min = 0.0;
		Max = 1.0;
		bValid = true;
		return;
	}

	if (Max < Min)
	{
		Swap(Min, Max);
	}

	if (FMath::IsNearlyEqual(Min, Max))
	{
		// 所有数据点相等时也要能画出来：按量级撑开一个可见跨度
		const double Span = FMath::Max(1.0, FMath::Abs(Min)) * FMath::Max(MinSpanRatio, 1e-3);
		Min -= Span;
		Max += Span;
	}
}

void FChartDomain::Pad(double Ratio)
{
	if (!IsValid() || Ratio <= 0.0)
	{
		return;
	}

	const double Delta = Size() * Ratio;
	Min -= Delta;
	Max += Delta;
}

bool FChartDomain::Contains(double Value) const
{
	return bValid && Value >= Min && Value <= Max;
}

FChartDomain FChartDomain::FromRange(double InMin, double InMax)
{
	FChartDomain Domain;
	if (FMath::IsFinite(InMin) && FMath::IsFinite(InMax))
	{
		Domain.Min = FMath::Min(InMin, InMax);
		Domain.Max = FMath::Max(InMin, InMax);
		Domain.bValid = true;
		Domain.Normalize();
	}
	return Domain;
}

// ---------------------------------------------------------------------------- ChartDomainUtil

bool ChartDomainUtil::ClampDomainToRange(FChartDomain& Domain, double Lower, double Upper, const FChartDomain& Current)
{
	if (Upper - Lower <= KINDA_SMALL_NUMBER)
	{
		// 限制区间本身无效，不夹
		return true;
	}

	Domain.Min = FMath::Max(Domain.Min, Lower);
	Domain.Max = FMath::Min(Domain.Max, Upper);

	if (Domain.Max - Domain.Min <= KINDA_SMALL_NUMBER)
	{
		// 整体被夹没了：退化成展示整个限制区间
		Domain.Min = Lower;
		Domain.Max = Upper;
	}

	Domain.bValid = true;

	// 与「当前显示域」相比没有任何变化，说明已经顶死在边界
	const double Tolerance = FMath::Max(KINDA_SMALL_NUMBER, (Upper - Lower) * 1e-9);
	const bool bUnchanged = FMath::Abs(Domain.Min - Current.Min) <= Tolerance
		&& FMath::Abs(Domain.Max - Current.Max) <= Tolerance;

	return !bUnchanged;
}

// ---------------------------------------------------------------------------- FChartCartesianSpace

void FChartCartesianSpace::Setup(const FChartDomain& InXDomain, const FChartDomain& InYDomain, const FBox2D& InPlotRect)
{
	XDomain = InXDomain;
	YDomain = InYDomain;
	PlotRect = InPlotRect;

	// 退化域在这里统一修正，调用方不需要各自处理
	XDomain.Normalize();
	YDomain.Normalize();
}

bool FChartCartesianSpace::IsValid() const
{
	return XDomain.IsValid() && YDomain.IsValid()
		&& PlotRect.Max.X > PlotRect.Min.X
		&& PlotRect.Max.Y > PlotRect.Min.Y;
}

double FChartCartesianSpace::DataXToLocal(double X) const
{
	if (!IsValid())
	{
		return PlotRect.Min.X;
	}

	const double Width = PlotRect.Max.X - PlotRect.Min.X;
	return PlotRect.Min.X + (X - XDomain.Min) / XDomain.Size() * Width;
}

double FChartCartesianSpace::DataYToLocal(double Y) const
{
	if (!IsValid())
	{
		return PlotRect.Max.Y;
	}

	// 局部坐标 Y 向下，数据 Y 向上
	const double Height = PlotRect.Max.Y - PlotRect.Min.Y;
	return PlotRect.Max.Y - (Y - YDomain.Min) / YDomain.Size() * Height;
}

FVector2D FChartCartesianSpace::DataToLocal(const FVector2D& Data) const
{
	return FVector2D(DataXToLocal(Data.X), DataYToLocal(Data.Y));
}

FVector2D FChartCartesianSpace::LocalToData(const FVector2D& Local) const
{
	if (!IsValid())
	{
		return FVector2D(XDomain.Min, YDomain.Min);
	}

	const double Width = PlotRect.Max.X - PlotRect.Min.X;
	const double Height = PlotRect.Max.Y - PlotRect.Min.Y;

	const double DataX = XDomain.Min + (Local.X - PlotRect.Min.X) / Width * XDomain.Size();
	const double DataY = YDomain.Min + (PlotRect.Max.Y - Local.Y) / Height * YDomain.Size();
	return FVector2D(DataX, DataY);
}
