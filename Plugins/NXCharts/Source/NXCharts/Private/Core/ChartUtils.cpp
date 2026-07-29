// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ChartUtils.h"

float ChartUtils::PointToSegmentDistance(const FVector2D& Point, const FVector2D& A, const FVector2D& B, FVector2D& OutClosestPoint)
{
	FVector2D AB = B - A;
	float LengthSq = AB.SizeSquared();
	if (LengthSq < KINDA_SMALL_NUMBER)
	{
		OutClosestPoint = A;
		return FVector2D::Distance(Point, A);
	}
	float T = FMath::Clamp(FVector2D::DotProduct(Point - A, AB) / LengthSq, 0.0f, 1.0f);
	OutClosestPoint = A + T * AB;
	return FVector2D::Distance(Point, OutClosestPoint);
}