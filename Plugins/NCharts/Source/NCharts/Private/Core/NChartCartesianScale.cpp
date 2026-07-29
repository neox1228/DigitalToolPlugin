// Copyright NCharts Plugin. All Rights Reserved.

#include "Core/NChartCartesianScale.h"

namespace
{
	float NiceNumber(float Value, bool bRound)
	{
		const float Exp = FMath::FloorToFloat(FMath::Loge(Value) / FMath::Loge(10.0f));
		const float Fraction = Value / FMath::Pow(10.0f, Exp);
		float NiceFraction = 1.0f;

		if (bRound)
		{
			if (Fraction < 1.5f)
			{
				NiceFraction = 1.0f;
			}
			else if (Fraction < 3.0f)
			{
				NiceFraction = 2.0f;
			}
			else if (Fraction < 7.0f)
			{
				NiceFraction = 5.0f;
			}
			else
			{
				NiceFraction = 10.0f;
			}
		}
		else
		{
			if (Fraction <= 1.0f)
			{
				NiceFraction = 1.0f;
			}
			else if (Fraction <= 2.0f)
			{
				NiceFraction = 2.0f;
			}
			else if (Fraction <= 5.0f)
			{
				NiceFraction = 5.0f;
			}
			else
			{
				NiceFraction = 10.0f;
			}
		}

		return NiceFraction * FMath::Pow(10.0f, Exp);
	}
}

void FNChartCartesianScale::CalcNiceScale(float InMin, float InMax, int32 MaxTicks, float& OutMin, float& OutMax, float& OutStep)
{
	if (InMax < InMin)
	{
		Swap(InMax, InMin);
	}

	if (FMath::IsNearlyEqual(InMin, InMax))
	{
		if (FMath::IsNearlyZero(InMin))
		{
			OutMin = 0.0f;
			OutMax = 1.0f;
			OutStep = 0.2f;
			return;
		}

		const float Padding = FMath::Max(FMath::Abs(InMin) * 0.1f, 1.0f);
		InMin -= Padding;
		InMax += Padding;
	}

	const float Range = NiceNumber(InMax - InMin, false);
	OutStep = NiceNumber(Range / FMath::Max(MaxTicks - 1, 1), true);
	OutMin = FMath::FloorToFloat(InMin / OutStep) * OutStep;
	OutMax = FMath::CeilToFloat(InMax / OutStep) * OutStep;
}

void FNChartCartesianScale::BuildValueDomainFromExtent(float ExtentMin, float ExtentMax, FNChartAxisDomain& InOutDomain)
{
	float NiceMin = ExtentMin;
	float NiceMax = ExtentMax;
	float Step = 1.0f;

	if (InOutDomain.bAutoMin || InOutDomain.bAutoMax)
	{
		CalcNiceScale(ExtentMin, ExtentMax, InOutDomain.SplitCount, NiceMin, NiceMax, Step);
	}

	InOutDomain.Min = InOutDomain.bAutoMin ? NiceMin : InOutDomain.Min;
	InOutDomain.Max = InOutDomain.bAutoMax ? NiceMax : InOutDomain.Max;

	if (InOutDomain.Max <= InOutDomain.Min)
	{
		InOutDomain.Max = InOutDomain.Min + 1.0f;
	}
}

void FNChartCartesianScale::UpdatePixelRect(const FVector2D& InPadding, const FVector2D& GeometrySize)
{
	Padding = InPadding;
	DrawMin = Padding;
	DrawMax = FVector2D(GeometrySize.X - Padding.X, GeometrySize.Y - Padding.Y);
	BuildTicks();
}

void FNChartCartesianScale::BuildTicks()
{
	XTicks.Reset();
	YTicks.Reset();

	const float DrawWidth = DrawMax.X - DrawMin.X;
	const float DrawHeight = DrawMax.Y - DrawMin.Y;
	if (DrawWidth <= 1.0f || DrawHeight <= 1.0f)
	{
		return;
	}

	if (XDomain.Type == EAxisType::Value)
	{
		float Step = 1.0f;
		float NiceMin = XDomain.Min;
		float NiceMax = XDomain.Max;
		CalcNiceScale(XDomain.Min, XDomain.Max, XDomain.SplitCount, NiceMin, NiceMax, Step);

		for (float Value = NiceMin; Value <= NiceMax + Step * 0.5f; Value += Step)
		{
			const float NormalX = (Value - XDomain.Min) / FMath::Max(XDomain.Max - XDomain.Min, KINDA_SMALL_NUMBER);
			FNChartAxisTick Tick;
			Tick.DataValue = Value;
			Tick.Label = FText::AsNumber(Value);
			Tick.ScreenPos = DrawMin.X + NormalX * DrawWidth;
			XTicks.Add(Tick);
		}
	}
	else if (XDomain.Categories.Num() > 0)
	{
		const int32 Count = XDomain.Categories.Num();
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const float NormalX = Count > 1
				? static_cast<float>(Index) / static_cast<float>(Count - 1)
				: 0.5f;

			FNChartAxisTick Tick;
			Tick.CategoryIndex = Index;
			Tick.DataValue = static_cast<float>(Index);
			Tick.Label = FText::FromString(XDomain.Categories[Index]);
			Tick.ScreenPos = DrawMin.X + NormalX * DrawWidth;
			XTicks.Add(Tick);
		}
	}

	if (YDomain.Type == EAxisType::Value)
	{
		float Step = 1.0f;
		float NiceMin = YDomain.Min;
		float NiceMax = YDomain.Max;
		CalcNiceScale(YDomain.Min, YDomain.Max, YDomain.SplitCount, NiceMin, NiceMax, Step);

		for (float Value = NiceMin; Value <= NiceMax + Step * 0.5f; Value += Step)
		{
			const float NormalY = (Value - YDomain.Min) / FMath::Max(YDomain.Max - YDomain.Min, KINDA_SMALL_NUMBER);
			FNChartAxisTick Tick;
			Tick.DataValue = Value;
			Tick.Label = FText::AsNumber(Value);
			Tick.ScreenPos = DrawMax.Y - NormalY * DrawHeight;
			YTicks.Add(Tick);
		}
	}
	else if (YDomain.Categories.Num() > 0)
	{
		const int32 Count = YDomain.Categories.Num();
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const float NormalY = Count > 1
				? static_cast<float>(Index) / static_cast<float>(Count - 1)
				: 0.5f;

			FNChartAxisTick Tick;
			Tick.CategoryIndex = Index;
			Tick.DataValue = static_cast<float>(Index);
			Tick.Label = FText::FromString(YDomain.Categories[Index]);
			Tick.ScreenPos = DrawMax.Y - NormalY * DrawHeight;
			YTicks.Add(Tick);
		}
	}
}

float FNChartCartesianScale::DataXToScreen(float DataX) const
{
	const float DrawWidth = DrawMax.X - DrawMin.X;
	if (DrawWidth <= KINDA_SMALL_NUMBER)
	{
		return DrawMin.X;
	}

	if (XDomain.Type == EAxisType::Category)
	{
		const int32 Count = XDomain.Categories.Num();
		if (Count <= 0)
		{
			return DrawMin.X;
		}

		const float ClampedIndex = FMath::Clamp(DataX, 0.0f, static_cast<float>(Count - 1));
		const float NormalX = Count > 1
			? ClampedIndex / static_cast<float>(Count - 1)
			: 0.5f;
		return DrawMin.X + NormalX * DrawWidth;
	}

	const float NormalX = (DataX - XDomain.Min) / FMath::Max(XDomain.Max - XDomain.Min, KINDA_SMALL_NUMBER);
	return DrawMin.X + NormalX * DrawWidth;
}

float FNChartCartesianScale::DataYToScreen(float DataY) const
{
	const float DrawHeight = DrawMax.Y - DrawMin.Y;
	if (DrawHeight <= KINDA_SMALL_NUMBER)
	{
		return DrawMax.Y;
	}

	if (YDomain.Type == EAxisType::Category)
	{
		const int32 Count = YDomain.Categories.Num();
		if (Count <= 0)
		{
			return DrawMax.Y;
		}

		const float ClampedIndex = FMath::Clamp(DataY, 0.0f, static_cast<float>(Count - 1));
		const float NormalY = Count > 1
			? ClampedIndex / static_cast<float>(Count - 1)
			: 0.5f;
		return DrawMax.Y - NormalY * DrawHeight;
	}

	const float NormalY = (DataY - YDomain.Min) / FMath::Max(YDomain.Max - YDomain.Min, KINDA_SMALL_NUMBER);
	return DrawMax.Y - NormalY * DrawHeight;
}

FVector2D FNChartCartesianScale::DataToScreen(const FVector2D& DataPoint) const
{
	return FVector2D(DataXToScreen(DataPoint.X), DataYToScreen(DataPoint.Y));
}

bool FNChartCartesianScale::ScreenXToDataX(float ScreenX, float& OutDataX) const
{
	const float DrawWidth = DrawMax.X - DrawMin.X;
	if (DrawWidth <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float ClampedX = FMath::Clamp(ScreenX, DrawMin.X, DrawMax.X);
	const float NormalX = (ClampedX - DrawMin.X) / DrawWidth;

	if (XDomain.Type == EAxisType::Category)
	{
		const int32 Count = XDomain.Categories.Num();
		if (Count <= 0)
		{
			return false;
		}

		OutDataX = NormalX * FMath::Max(Count - 1, 1);
		return true;
	}

	OutDataX = XDomain.Min + NormalX * (XDomain.Max - XDomain.Min);
	return true;
}

bool FNChartCartesianScale::ScreenYToDataY(float ScreenY, float& OutDataY) const
{
	const float DrawHeight = DrawMax.Y - DrawMin.Y;
	if (DrawHeight <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float ClampedY = FMath::Clamp(ScreenY, DrawMin.Y, DrawMax.Y);
	const float NormalY = (DrawMax.Y - ClampedY) / DrawHeight;

	if (YDomain.Type == EAxisType::Category)
	{
		const int32 Count = YDomain.Categories.Num();
		if (Count <= 0)
		{
			return false;
		}

		OutDataY = NormalY * FMath::Max(Count - 1, 1);
		return true;
	}

	OutDataY = YDomain.Min + NormalY * (YDomain.Max - YDomain.Min);
	return true;
}

FVector2D FNChartCartesianScale::ScreenToData(const FVector2D& ScreenPoint) const
{
	float DataX = 0.0f;
	float DataY = 0.0f;
	ScreenXToDataX(ScreenPoint.X, DataX);
	ScreenYToDataY(ScreenPoint.Y, DataY);
	return FVector2D(DataX, DataY);
}

void FNChartCartesianScale::BuildScreenPoints(const TArray<FVector2D>& DataPoints, TArray<FVector2D>& OutScreenPoints) const
{
	OutScreenPoints.Reset();
	OutScreenPoints.Reserve(DataPoints.Num());
	for (const FVector2D& Point : DataPoints)
	{
		OutScreenPoints.Add(DataToScreen(Point));
	}
}
