// Fill out your copyright notice in the Description page of Project Settings.


#include "Feature/Components/LineSeriesComponent.h"
#include "Core/ChartDrawContext.h"
#include "Feature/Modules/CartesianCoordModule.h"
#include "Core/ChartUtils.h"

void UlineSeriesComponent::CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry)
{
	TArray<FVector2D> Points = MapDataToPixel(AllottedGeometry);
	if (Points.Num() < 2) return;
	Ctx.DrawLine(Points, LineColor, LineWidth, true);
	
}

int32 UlineSeriesComponent::HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const
{
	const TArray<FVector2D> Points = MapDataToPixel(AllottedGeometry); // 直接调用，无需转换
	const float Threshold = 10.0f;
	for (int32 i = 1; i < Points.Num(); ++i)
	{
		FVector2D ClosestPt;
		float Dist = ChartUtils::PointToSegmentDistance(LocalPos, Points[i-1], Points[i], ClosestPt);
		if (Dist <= Threshold) return i - 1;
	}
	return -1;
}

TArray<FVector2D> UlineSeriesComponent::MapDataToPixel(const FGeometry& AllottedGeometry) const
{
	TArray<FVector2D> Pixels;
	UChartModule* Mod = GetParentModule();
	UCartesianCoordModule* CoordMod = Cast<UCartesianCoordModule>(Mod);
	if (!CoordMod) return Pixels;

	FBox2D PlotBounds = CoordMod->GetPaddedPlotBounds(AllottedGeometry);
	FVector2D XRange = CoordMod->CachedXRange.IsZero() ? CoordMod->XAxisRange : CoordMod->CachedXRange;
	FVector2D YRange = CoordMod->CachedYRange.IsZero() ? CoordMod->YAxisRange : CoordMod->CachedYRange;

	for (const FVector2D& Pt : DataPoints)
	{
		float Px = ChartUtils::MapValue(Pt.X, XRange.X, XRange.Y, PlotBounds.Min.X, PlotBounds.Max.X);
		float Py = ChartUtils::MapValue(Pt.Y, YRange.X, YRange.Y, PlotBounds.Max.Y, PlotBounds.Min.Y); // 注意Y轴翻转
		Pixels.Add(FVector2D(Px, Py));
	}
	return Pixels;
}
