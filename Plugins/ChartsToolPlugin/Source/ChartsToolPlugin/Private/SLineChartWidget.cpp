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
	SeriesSetting = InArgs._SeriesSetting;
}

int32 SLineChartWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	
	TArray<FVector2D> Points = Data.Get();
	FChartMath::SortByValueX(Points);
	
	if (Points.Num() < 2)
	{
		return LayerId;
	}

	
	
	FAxisLayout AxisSettings = AxisLayout.Get() ;

	

	
	if (bCanUpdate)
	{
		RenderPoints.Reset();
		for (int32 i = 0; i < Points.Num(); i++)
		{
			FVector2D LocalPoint = FChartMath::DataToLocal(Points[i],AllottedGeometry.GetLocalSize(),AxisSettings);
			RenderPoints.Add(LocalPoint);
		}
		if (SeriesSetting.Get().Smooth)
		{
			TArray<FVector2D> SmoothPoints;
			FChartMath::CalculateCatmullRomPoints(RenderPoints, SmoothPoints);
			RenderPoints.Reset();
			RenderPoints = SmoothPoints;
		}
		UE_LOG(LogTemp, Warning, TEXT("neo---数据更新"));
		bCanUpdate = false;
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
