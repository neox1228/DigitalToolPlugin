// Fill out your copyright notice in the Description page of Project Settings.


#include "SLineChartWidget.h"


#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLineChartWidget::Construct(const FArguments& InArgs)
{
	Data = InArgs._Data;
	LineColor = InArgs._LineColor;
	LineThinckness = InArgs._LineThinckness;
}

int32 SLineChartWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	TArray<FVector2D> Points = Data.Get();
	//UE_LOG(LogTemp, Warning, TEXT("neo---折线图大小：Size_X:%d; Size_Y:%d"), AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y);

	if (Points.Num() < 2)
	{
		return LayerId;
	}

	FVector2D Size = AllottedGeometry.GetLocalSize();
	FVector2D OutMin;
	FVector2D OutMax;
	FChartMath::GetArrayRange(Points,OutMin,OutMax);
	FChartAxisSettings AxisSettings ;
	AxisSettings.Origin = Origin.Get();
	AxisSettings.XAxisMax = OutMax.X;
	AxisSettings.XAxisMin = OutMin.X;
	AxisSettings.YAxisMax = OutMax.Y;
	AxisSettings.YAxisMin = OutMin.Y;
 
	for (int32 i = 0; i < Points.Num() - 1; i++)
	{
		FVector2D P1 = FChartMath::DataToLocal(Points[i],AllottedGeometry, AxisSettings);
		FVector2D P2 = FChartMath::DataToLocal(Points[i + 1],AllottedGeometry, AxisSettings);
		FSlateDrawElement::MakeLines(OutDrawElements,LayerId,AllottedGeometry.ToPaintGeometry(),{P1, P2},ESlateDrawEffect::None, LineColor.Get(),true
			,LineThinckness.Get());
	}
	return LayerId + 1;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
