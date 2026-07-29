// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace ChartUtils
{
	// 将数据值映射到像素坐标（线型映射）
	inline float MapValue(float Value, float InMin, float InMax, float OutMin, float OutMax)
	{
		if (FMath::IsNearlyEqual(InMax, InMin)) return OutMin;
		return OutMin + (Value - InMin) / (InMax - InMin) * (OutMax - OutMin);
	}

	// 获取两点间距离
	inline float Distance(const FVector2D& A, const FVector2D& B)
	{
		return FVector2D::Distance(A, B);
	}

	// 获取点到线段的最近距离和对应点
	// （可用于命中测试，这里只给出声明，实现放在 .cpp 里）
	float PointToSegmentDistance(const FVector2D& Point, const FVector2D& A, const FVector2D& B, FVector2D& OutClosestPoint);
}
