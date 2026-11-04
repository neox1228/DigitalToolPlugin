// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartSeriesBuffer.h
 * @brief   高频数据通道：按 SeriesKey 存点集，与 Spec 完全分离。
 *
 * 为什么不把点集放进 Spec 或 UObject 属性：
 *  - 属性系统会做整体深拷贝，1 万点 x 20Hz 直接压垮
 *  - Revision 让几何缓存可以精确失效（没变就不重建）
 *  - Capacity > 0 时按块裁剪队首，把 RemoveAt 的 O(N) 摊薄到每次追加
 */

#pragma once

#include "CoreMinimal.h"

/** 单条系列的数据缓冲 */
struct NXCHARTS_API FChartSeriesBuffer
{
	TArray<FVector2d> Points;

	/** 每次内容变化 +1，供上层判断几何是否需要重建 */
	int32 Revision = 0;

	/** <= 0 表示不限制；> 0 时保留最近 Capacity 个点（实时波形用滑动窗口） */
	int32 Capacity = 0;

	void Reset();
	void SetPoints(const TArray<FVector2d>& InPoints);
	void SetPoints(TArray<FVector2d>&& InPoints);
	void AddPoint(const FVector2d& Point);
	void AddPoints(TArrayView<const FVector2d> InPoints);

	/** 超出 Capacity 时裁剪队首 */
	void TrimToCapacity();

	int32 Num() const { return Points.Num(); }
	const TArray<FVector2d>& GetPoints() const { return Points; }

	void MarkModified() { ++Revision; }
};

/** 所有系列的数据容器，按 SeriesKey 索引 —— 与 Spec 的 SeriesKey 对齐 */
struct NXCHARTS_API FChartSeriesDataPool
{
	TMap<FName, FChartSeriesBuffer> Buffers;

	FChartSeriesBuffer& FindOrAdd(FName Key);
	const FChartSeriesBuffer* Find(FName Key) const;
	FChartSeriesBuffer* FindMutable(FName Key);

	bool Remove(FName Key);
	void Reset();

	int32 Num() const { return Buffers.Num(); }

	/** 所有系列 Revision 之和，用于帧缓存校验 */
	int64 GetRevisionSum() const;
};