// Fill out your copyright notice in the Description page of Project Settings.


#include "Layout/ChartTicks.h"

void FChartTickSet::Reset()
{
	Ticks.Reset();
	Step = 0.0;
}

int32 FChartTickSet::NumMajor() const
{
	int32 Count = 0;
	for (const FChartTick& Tick : Ticks)
	{
		if (Tick.bMajor) ++Count;
	}
	return Count;
}

int32 FChartTickSet::NumMinor() const
{
	return Ticks.Num() - NumMajor();
}

namespace ChartTicks
{
	namespace
	{
		constexpr double Log10E = 2.302585092994046;

		double Log10(double Value)
		{
			return FMath::Loge(Value) / Log10E;
		}
	}

	double NiceStep(double Range, int32 DesiredIntervals)
	{
		if (Range <= 0.0 || DesiredIntervals <= 0)
		{
			return 0.0;
		}

		const double Raw = Range / static_cast<double>(DesiredIntervals);
		const double Exponent = FMath::FloorToDouble(Log10(Raw));
		const double Power = FMath::Pow(10.0, Exponent);
		const double Fraction = Raw / Power;

		double NiceMultiplier = 10.0;
		if (Fraction <= 1.0)
		{
			NiceMultiplier = 1.0;
		}
		else if (Fraction <= 2.0)
		{
			NiceMultiplier = 2.0;
		}
		else if (Fraction <= 5.0)
		{
			NiceMultiplier = 5.0;
		}

		return NiceMultiplier * Power;
	}

	int32 DecimalsForStep(double Step)
	{
		if (Step <= 0.0)
		{
			return 2;
		}

		const double Exponent = FMath::FloorToDouble(Log10(Step));
		return static_cast<int32>(FMath::Clamp(-Exponent, 0.0, 6.0));
	}

	FString FormatValue(double Value, double Step, EChartTickFormat Format)
	{
		// 抹掉 -0 和浮点累加噪声
		const double Tolerance = FMath::Max(FMath::Abs(Step), 1e-9) * 1e-6;
		if (FMath::Abs(Value) < Tolerance)
		{
			Value = 0.0;
		}

		switch (Format)
		{
		case EChartTickFormat::Integer:
			return FString::Printf(TEXT("%lld"), static_cast<int64>(FMath::RoundToDouble(Value)));

		case EChartTickFormat::OneDecimal:
			return FString::Printf(TEXT("%.1f"), Value);

		case EChartTickFormat::TwoDecimals:
			return FString::Printf(TEXT("%.2f"), Value);

		case EChartTickFormat::Percent:
			return FString::Printf(TEXT("%.1f%%"), Value * 100.0);

		case EChartTickFormat::Scientific:
			return FString::Printf(TEXT("%.2e"), Value);

		case EChartTickFormat::Auto:
		default:
			return FString::Printf(TEXT("%.*f"), DecimalsForStep(Step), Value);
		}
	}

	FChartTickSet MakeLinearTicks(const FChartDomain& Domain, int32 DesiredTickCount,
	                              EChartTickFormat Format, bool bIncludeMinor)
	{
		FChartTickSet Result;
		if (!Domain.IsValid() || DesiredTickCount < 2)
		{
			return Result;
		}

		const int32 Intervals = DesiredTickCount - 1;
		const double Step = NiceStep(Domain.Size(), Intervals);
		if (Step <= 0.0 || !FMath::IsFinite(Step))
		{
			return Result;
		}

		// 主刻度数量不多时才插入次刻度，否则视觉上会糊成一片
		const int32 Subdivisions = (bIncludeMinor && DesiredTickCount <= 12) ? 5 : 1;
		const double FineStep = Step / static_cast<double>(Subdivisions);

		const int64 FirstIndex = static_cast<int64>(FMath::CeilToDouble(Domain.Min / FineStep - 1e-9));
		const int64 LastIndex = static_cast<int64>(FMath::FloorToDouble(Domain.Max / FineStep + 1e-9));
		if (LastIndex < FirstIndex || (LastIndex - FirstIndex) > 2000)
		{
			return Result;
		}

		Result.Step = Step;
		Result.Ticks.Reserve(static_cast<int32>(LastIndex - FirstIndex) + 1);

		for (int64 Index = FirstIndex; Index <= LastIndex; ++Index)
		{
			const bool bIsMajor = (Subdivisions == 1) || ((Index % Subdivisions) == 0);

			FChartTick Tick;
			Tick.Value = static_cast<double>(Index) * FineStep;
			Tick.bMajor = bIsMajor;
			if (bIsMajor)
			{
				Tick.Label = FormatValue(Tick.Value, Step, Format);
			}
			Result.Ticks.Emplace(MoveTemp(Tick));
		}

		return Result;
	}
}
