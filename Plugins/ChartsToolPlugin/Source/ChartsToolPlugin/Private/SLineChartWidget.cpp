// Fill out your copyright notice in the Description page of Project Settings.


#include "SLineChartWidget.h"


#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLineChartWidget::Construct(const FArguments& InArgs)
{
	Data = InArgs._Data;
	LineColor = InArgs._LineColor;
	CustomBoundX = InArgs._CustomBoundX;
	CustomBoundY = InArgs._CustomBoundY;
	MaxX = InArgs._MaxX;
	MaxY = InArgs._MaxY;
	MinX = InArgs._MinX;
	MinY = InArgs._MinY;
	LineThinckness = InArgs._LineThinckness;
}

int32 SLineChartWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	TArray<FVector2D> Points = Data.Get();
	FChartMath::SortByValueX(Points);
	//UE_LOG(LogTemp, Warning, TEXT("neo---折线图大小：Size_X:%d; Size_Y:%d"), AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y);

	if (Points.Num() < 2)
	{
		return LayerId;
	}

	
	FAxisLayout AxisSettings = AxisLayout.Get() ;
	

	TArray<FVector2D> RenderPoints;
	for (int32 i = 0; i < Points.Num(); i++)
	{
		FVector2D LocalPoint = FChartMath::DataToLocal(Points[i],AllottedGeometry.GetLocalSize(),AxisSettings);

		LocalPoint.X = FMath::RoundToFloat(LocalPoint.X) + 0.5f;
		LocalPoint.Y = FMath::RoundToFloat(LocalPoint.Y) + 0.5f;
		
		RenderPoints.Add(LocalPoint);
		//FSlateDrawElement::make
		
	}
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,AllottedGeometry.ToPaintGeometry(),
		RenderPoints, ESlateDrawEffect::None,
		LineColor.Get(),
		true,
		LineThinckness.Get());
	return LayerId + 1;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
