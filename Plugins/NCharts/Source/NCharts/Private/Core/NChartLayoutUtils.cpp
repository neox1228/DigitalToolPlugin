// Copyright NCharts Plugin. All Rights Reserved.

#include "Core/NChartLayoutUtils.h"

bool FNChartLayoutUtils::BuildScreenLayout(
	const TArray<FVector2D>& DataPoints,
	const FVector2D& Padding,
	const FVector2D& GeometrySize,
	int32 MinPointCount,
	FNChartScreenLayout& OutLayout)
{
	if (DataPoints.Num() < MinPointCount)
	{
		return false;
	}

	OutLayout.DrawMin = Padding;
	OutLayout.DrawMax = FVector2D(GeometrySize.X - Padding.X, GeometrySize.Y - Padding.Y);
	const FVector2D DrawSize = OutLayout.DrawMax - OutLayout.DrawMin;
	if (DrawSize.X <= 1.0f || DrawSize.Y <= 1.0f)
	{
		return false;
	}

	OutLayout.DataMin = FVector2D(FLT_MAX, FLT_MAX);
	OutLayout.DataMax = FVector2D(-FLT_MAX, -FLT_MAX);
	for (const FVector2D& Point : DataPoints)
	{
		OutLayout.DataMin.X = FMath::Min(OutLayout.DataMin.X, Point.X);
		OutLayout.DataMin.Y = FMath::Min(OutLayout.DataMin.Y, Point.Y);
		OutLayout.DataMax.X = FMath::Max(OutLayout.DataMax.X, Point.X);
		OutLayout.DataMax.Y = FMath::Max(OutLayout.DataMax.Y, Point.Y);
	}

	const float RangeX = FMath::Max(OutLayout.DataMax.X - OutLayout.DataMin.X, 1.0f);
	const float RangeY = FMath::Max(OutLayout.DataMax.Y - OutLayout.DataMin.Y, 1.0f);

	OutLayout.ScreenPoints.Reset();
	OutLayout.ScreenPoints.Reserve(DataPoints.Num());
	for (const FVector2D& Point : DataPoints)
	{
		const float NormalX = (Point.X - OutLayout.DataMin.X) / RangeX;
		const float NormalY = (Point.Y - OutLayout.DataMin.Y) / RangeY;
		OutLayout.ScreenPoints.Add(FVector2D(
			OutLayout.DrawMin.X + NormalX * DrawSize.X,
			OutLayout.DrawMax.Y - NormalY * DrawSize.Y));
	}

	return true;
}

bool FNChartLayoutUtils::ScreenXToDataX(float ScreenX, const FNChartScreenLayout& Layout, float& OutDataX)
{
	const float DrawWidth = Layout.DrawMax.X - Layout.DrawMin.X;
	if (DrawWidth <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float ClampedX = FMath::Clamp(ScreenX, Layout.DrawMin.X, Layout.DrawMax.X);
	const float NormalX = (ClampedX - Layout.DrawMin.X) / DrawWidth;
	const float RangeX = FMath::Max(Layout.DataMax.X - Layout.DataMin.X, 1.0f);
	OutDataX = Layout.DataMin.X + NormalX * RangeX;
	return true;
}

int32 FNChartLayoutUtils::FindNearestByScreenPosition(
	const TArray<FVector2D>& ScreenPoints,
	const FVector2D& MousePosition,
	float& OutDistanceSquared)
{
	if (ScreenPoints.Num() == 0)
	{
		return INDEX_NONE;
	}

	int32 BestIndex = INDEX_NONE;
	OutDistanceSquared = TNumericLimits<float>::Max();

	for (int32 Index = 0; Index < ScreenPoints.Num(); ++Index)
	{
		const float DistanceSquared = FVector2D::DistSquared(ScreenPoints[Index], MousePosition);
		if (DistanceSquared < OutDistanceSquared)
		{
			OutDistanceSquared = DistanceSquared;
			BestIndex = Index;
		}
	}

	return BestIndex;
}

int32 FNChartLayoutUtils::FindNearestByDataX(const TArray<FVector2D>& DataPoints, float DataX)
{
	if (DataPoints.Num() == 0)
	{
		return INDEX_NONE;
	}

	int32 BestIndex = INDEX_NONE;
	float BestDistance = TNumericLimits<float>::Max();

	for (int32 Index = 0; Index < DataPoints.Num(); ++Index)
	{
		const float Distance = FMath::Abs(DataPoints[Index].X - DataX);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestIndex = Index;
		}
	}

	return BestIndex;
}
