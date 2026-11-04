// Fill out your copyright notice in the Description page of Project Settings.


#include "Frame/ChartCurve.h"

namespace
{
	/** 每个像素段的目标采样长度，越小越圆滑也越费顶点 */
	constexpr double PixelsPerSample = 6.0;
	constexpr int32 MinSamplesPerSegment = 2;
	constexpr int32 MaxSamplesPerSegment = 16;

	/** 超过这个点数就不做样条了：段数太多，细分收益为负 */
	constexpr int32 MaxSmoothSourcePoints = 4000;
}

void ChartCurve::SmoothMonotone(const TArray<FVector2d>& In, int32 MaxOutputPoints, TArray<FVector2d>& Out)
{
	Out.Reset();

	const int32 Num = In.Num();
	if (Num < 3 || Num > MaxSmoothSourcePoints)
	{
		Out = In;
		return;
	}

	// 单调插值要求 X 非递减；乱序数据退化为折线，避免画出回头线
	for (int32 Index = 1; Index < Num; ++Index)
	{
		if (In[Index].X < In[Index - 1].X)
		{
			Out = In;
			return;
		}
	}

	const int32 SegmentCount = Num - 1;

	// 1) 区间斜率
	TArray<double> Slopes;
	Slopes.SetNum(SegmentCount);
	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		const double DeltaX = In[Index + 1].X - In[Index].X;
		Slopes[Index] = (DeltaX > KINDA_SMALL_NUMBER)
			? (In[Index + 1].Y - In[Index].Y) / DeltaX
			: 0.0;
	}

	// 2) 节点切线：中心差分
	TArray<double> Tangents;
	Tangents.SetNum(Num);
	Tangents[0] = Slopes[0];
	Tangents[Num - 1] = Slopes[SegmentCount - 1];
	for (int32 Index = 1; Index < Num - 1; ++Index)
	{
		Tangents[Index] = (Slopes[Index - 1] + Slopes[Index]) * 0.5;
	}

	// 3) Fritsch–Carlson 限制：保证不过冲
	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		if (FMath::IsNearlyZero(Slopes[Index]))
		{
			Tangents[Index] = 0.0;
			Tangents[Index + 1] = 0.0;
			continue;
		}

		const double Alpha = Tangents[Index] / Slopes[Index];
		const double Beta = Tangents[Index + 1] / Slopes[Index];
		const double Squared = Alpha * Alpha + Beta * Beta;
		if (Squared > 9.0)
		{
			const double Scale = 3.0 / FMath::Sqrt(Squared);
			Tangents[Index] = Scale * Alpha * Slopes[Index];
			Tangents[Index + 1] = Scale * Beta * Slopes[Index];
		}
	}

	// 4) 逐段采样
	Out.Reserve(FMath::Min(MaxOutputPoints, Num * MinSamplesPerSegment));
	Out.Add(In[0]);

	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		const double DeltaX = In[Index + 1].X - In[Index].X;
		const double SegmentLength = FVector2D::Distance(In[Index], In[Index + 1]);
		const int32 Samples = FMath::Clamp(
			FMath::CeilToInt(SegmentLength / PixelsPerSample), MinSamplesPerSegment, MaxSamplesPerSegment);

		for (int32 Sample = 1; Sample <= Samples; ++Sample)
		{
			const double T = static_cast<double>(Sample) / static_cast<double>(Samples);
			const double T2 = T * T;
			const double T3 = T2 * T;

			const double H00 = 2.0 * T3 - 3.0 * T2 + 1.0;
			const double H10 = T3 - 2.0 * T2 + T;
			const double H01 = -2.0 * T3 + 3.0 * T2;
			const double H11 = T3 - T2;

			// X 是参数本身（dx/dt = DeltaX），所以两端的 X 切线都等于 DeltaX
			const double X = H00 * In[Index].X + H10 * DeltaX + H01 * In[Index + 1].X + H11 * DeltaX;
			const double Y = H00 * In[Index].Y
				+ H10 * Tangents[Index] * DeltaX
				+ H01 * In[Index + 1].Y
				+ H11 * Tangents[Index + 1] * DeltaX;

			Out.Add(FVector2d(X, Y));

			if (Out.Num() >= MaxOutputPoints)
			{
				return;
			}
		}
	}
}

void ChartCurve::Step(const TArray<FVector2d>& In, TArray<FVector2d>& Out)
{
	Out.Reset();

	const int32 Num = In.Num();
	if (Num < 2)
	{
		Out = In;
		return;
	}

	Out.Reserve(Num * 2 - 1);
	Out.Add(In[0]);

	for (int32 Index = 1; Index < Num; ++Index)
	{
		// 先水平走到新的 X（保持上一个 Y），再竖直跳到新 Y
		Out.Add(FVector2d(In[Index].X, In[Index - 1].Y));
		Out.Add(In[Index]);
	}
}
