// Fill out your copyright notice in the Description page of Project Settings.


#include "Feature/Components/AxisComponent.h"
#include "Core/ChartDrawContext.h"
#include "Feature/Modules/ChartModule.h"

void UAxisComponent::CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry)
{
	TArray<FVector2D> Points = CalculateAxisPoints(AllottedGeometry);
	Ctx.DrawLine(Points, AxisColor, AxisThickness, true);
}

int32 UAxisComponent::HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const
{
	return Super::HitTest(LocalPos, AllottedGeometry);
}

TArray<FVector2D> UAxisComponent::CalculateAxisPoints(const FGeometry& AllottedGeometry) const
{
	TArray<FVector2D> Points;
	if (AxisType == EChartAxisType::X)
	{
		//AllottedGeometry.GetLocalSize()
		FBox2D BoundsSize = GetParentModule()->GetPlotBounds(AllottedGeometry);
		float Ratio = FMath::CeilToInt(BoundsSize.Max.X - BoundsSize.Min.X)/(AxisRange.AxisMax - AxisRange.AxisMin));
	}
	return TArray<FVector2D>();
}
