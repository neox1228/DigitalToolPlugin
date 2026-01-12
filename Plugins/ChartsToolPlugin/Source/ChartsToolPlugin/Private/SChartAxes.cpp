// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChartAxes::Construct(const FArguments& InArgs)
{
	Origin = InArgs._Origin;
	AxisType = InArgs._AxisType;

	BoundX = InArgs._BoundX;
	BoundY = InArgs._BoundY;
	TickNum = InArgs._TickNum;
	
	AxisColor = InArgs._AxisColor;
	AxisThickness = InArgs._AxisThickness;

	ShowGrid = InArgs._ShowGrid;
	XAxisTicks = InArgs._XAxisTicks;
	YAxisTicks = InArgs._YAxisTicks;
	TickFontSize = InArgs._TickFontSize;
	GridDensity = InArgs._GridDensity;
	
}

int32 SChartAxes::OnPaint(const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	if (ShowGrid.Get())
	{
		float MeridianNum = AllottedGeometry.GetLocalSize().X/GridDensity.Get();
		float ParallelNum = AllottedGeometry.GetLocalSize().Y/GridDensity.Get();
		for (int32 i = 0; i < MeridianNum; ++i)
		{
			//网格背景
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId ,
				AllottedGeometry.ToPaintGeometry(),
				{FVector2D{AllottedGeometry.GetLocalSize().X * (i / MeridianNum), 0}, FVector2D{AllottedGeometry.GetLocalSize().X * (i / MeridianNum), AllottedGeometry.GetLocalSize().Y}},
				ESlateDrawEffect::None,
				FLinearColor::Black,
				false,
				1.0f
				);
		}
		for (int32 i = 0; i < ParallelNum; ++i)
		{
			//网格背景
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				{FVector2D{0, AllottedGeometry.GetLocalSize().Y * (i / ParallelNum)}, FVector2D{ AllottedGeometry.GetLocalSize().X,AllottedGeometry.GetLocalSize().Y * (i / ParallelNum)}},
				ESlateDrawEffect::None,
				FLinearColor::Black,
				false,
				1.0f
				);
		}

	}
	
	// 获取坐标原点像素坐标值
	FVector2D OriginLocation = FChartMath::GetOriginLocation(Origin.Get(), AllottedGeometry);

	//坐标轴刻度绘制
	
	
	//X轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 3,
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
		LayerId + 3,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(OriginLocation.X, 0), OriginLocation, FVector2D(OriginLocation.X, AllottedGeometry.GetLocalSize().Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);
	

	
	
	return LayerId + 4;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
