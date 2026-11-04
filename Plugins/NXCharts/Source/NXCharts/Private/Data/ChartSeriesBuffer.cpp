// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ChartSeriesBuffer.h"

// ---------------------------------------------------------------------------- FChartSeriesBuffer

void FChartSeriesBuffer::Reset()
{
	Points.Reset();
	++Revision;
}

void FChartSeriesBuffer::SetPoints(const TArray<FVector2d>& InPoints)
{
	Points = InPoints;
	TrimToCapacity();
	++Revision;
}

void FChartSeriesBuffer::SetPoints(TArray<FVector2d>&& InPoints)
{
	Points = MoveTemp(InPoints);
	TrimToCapacity();
	++Revision;
}

void FChartSeriesBuffer::AddPoint(const FVector2d& Point)
{
	Points.Add(Point);
	TrimToCapacity();
	++Revision;
}

void FChartSeriesBuffer::AddPoints(TArrayView<const FVector2d> InPoints)
{
	if (InPoints.Num() == 0)
	{
		return;
	}

	Points.Append(InPoints.GetData(), InPoints.Num());
	TrimToCapacity();
	++Revision;
}

void FChartSeriesBuffer::TrimToCapacity()
{
	if (Capacity <= 0)
	{
		return;
	}

	const int32 Excess = Points.Num() - Capacity;
	if (Excess <= 0)
	{
		return;
	}

	// 一次多删一点（Capacity/8），把后续若干次追加的 O(N) 摊薄掉
	const int32 Chunk = FMath::Max(1, Capacity / 8);
	const int32 RemoveCount = FMath::Min(Excess + Chunk, FMath::Max(0, Points.Num() - 1));
	if (RemoveCount > 0)
	{
		Points.RemoveAt(0, RemoveCount);
	}
}

// ---------------------------------------------------------------------------- FChartSeriesDataPool

FChartSeriesBuffer& FChartSeriesDataPool::FindOrAdd(FName Key)
{
	return Buffers.FindOrAdd(Key);
}

const FChartSeriesBuffer* FChartSeriesDataPool::Find(FName Key) const
{
	return Buffers.Find(Key);
}

FChartSeriesBuffer* FChartSeriesDataPool::FindMutable(FName Key)
{
	return Buffers.Find(Key);
}

bool FChartSeriesDataPool::Remove(FName Key)
{
	return Buffers.Remove(Key) > 0;
}

void FChartSeriesDataPool::Reset()
{
	Buffers.Reset();
}

int64 FChartSeriesDataPool::GetRevisionSum() const
{
	int64 Sum = 0;
	for (const TPair<FName, FChartSeriesBuffer>& Pair : Buffers)
	{
		Sum += Pair.Value.Revision;
	}
	return Sum;
}
