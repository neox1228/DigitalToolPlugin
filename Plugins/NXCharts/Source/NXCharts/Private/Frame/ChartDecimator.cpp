// Fill out your copyright notice in the Description page of Project Settings.


#include "Frame/ChartDecimator.h"

void ChartDecimate::MinMaxByColumn(const TArray<FVector2d>& Points, double XMin, double XMax,
                                   double PlotWidth, int32 MaxOutputPoints, TArray<int32>& OutIndices)
{
	OutIndices.Reset();

	const int32 Num = Points.Num();
	if (Num == 0)
	{
		return;
	}

	const double Range = XMax - XMin;

	// 数据量不大 / 域退化 / 没有像素宽度 —— 不抽稀，直接全量
	if (Range <= 0.0 || PlotWidth <= 1.0 || Num <= MaxOutputPoints)
	{
		OutIndices.Reserve(Num);
		for (int32 Index = 0; Index < Num; ++Index)
		{
			OutIndices.Add(Index);
		}
		return;
	}

	const int32 Columns = FMath::Max(2, FMath::FloorToInt(PlotWidth));
	const double ColumnsPerUnit = static_cast<double>(Columns) / Range;

	int32 CurrentColumn = INDEX_NONE;
	int32 MinIndex = INDEX_NONE;
	int32 MaxIndex = INDEX_NONE;

	// 把当前列积累的 min/max 按索引升序写出，保证输出严格升序、连线顺序不乱
	auto FlushColumn = [&OutIndices, &MinIndex, &MaxIndex]()
	{
		if (MinIndex == INDEX_NONE)
		{
			return;
		}

		if (MinIndex == MaxIndex)
		{
			OutIndices.Add(MinIndex);
		}
		else if (MinIndex < MaxIndex)
		{
			OutIndices.Add(MinIndex);
			OutIndices.Add(MaxIndex);
		}
		else
		{
			OutIndices.Add(MaxIndex);
			OutIndices.Add(MinIndex);
		}

		MinIndex = INDEX_NONE;
		MaxIndex = INDEX_NONE;
	};

	for (int32 Index = 0; Index < Num; ++Index)
	{
		const FVector2d& Point = Points[Index];
		if (!FMath::IsFinite(Point.X) || !FMath::IsFinite(Point.Y))
		{
			continue;
		}

		int32 Column = FMath::FloorToInt((Point.X - XMin) * ColumnsPerUnit);
		Column = FMath::Clamp(Column, 0, Columns - 1);

		if (Column != CurrentColumn)
		{
			FlushColumn();
			CurrentColumn = Column;
			MinIndex = Index;
			MaxIndex = Index;
			continue;
		}

		if (Point.Y < Points[MinIndex].Y)
		{
			MinIndex = Index;
		}
		if (Point.Y > Points[MaxIndex].Y)
		{
			MaxIndex = Index;
		}
	}

	FlushColumn();
}
