// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChartAxes::Construct(const FArguments& InArgs)
{
	Origin = InArgs._Origin;
	
	AxisColor = InArgs._AxisColor;
	AxisThickness = InArgs._AxisThickness;

	ShowGrid = InArgs._ShowGrid;
	XAxisTicks = InArgs._XAxisTicks;
	YAxisTicks = InArgs._YAxisTicks;
	TickFontSize = InArgs._TickFontSize;
	
}

int32 SChartAxes::OnPaint(const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{

	// 获取坐标原点像素坐标值
	FVector2D OriginLocation = FChartMath::GetOriginLocation(Origin.Get(), AllottedGeometry);
	
	//X轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(0, OriginLocation.Y), OriginLocation, FVector2D(AllottedGeometry.GetLocalSize().X, OriginLocation.Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);

	//Y轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(OriginLocation.X, 0), OriginLocation, FVector2D(OriginLocation.X, AllottedGeometry.GetLocalSize().Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);
	
	return LayerId + 2;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
