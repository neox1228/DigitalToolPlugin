// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChartAxes::Construct(const FArguments& InArgs)
{
	Oringin = InArgs._Oringin;
	AxisColor = InArgs._AxisColor;
	AxisThickness = InArgs._AxisThickness;

	XAxisMin = InArgs._XAxisMin;
	XAxisMax = InArgs._XAxisMax;
	XAxisTicks = InArgs._XAxisTicks;

	
}

FVector2D SChartAxes::GetOriginPosition(const FVector2D& Size) const
{
	switch (Oringin.Get())
	{
	case EChartOrigin::LeftBottom: return FVector2D(0, Size.Y);
	case EChartOrigin::LeftCenter: return FVector2D(0, Size.Y * 0.5f);
	case EChartOrigin::LeftTop: return FVector2D(0, 0);
		
	case EChartOrigin::CenterBottom: return FVector2D(Size.X * 0.5, Size.Y);
	case EChartOrigin::Center: return FVector2D(Size.X * 0.5, Size.Y * 0.5);
	case EChartOrigin::CenterTop: return FVector2D(Size.X * 0.5, 0);

	case EChartOrigin::RightBottom: return FVector2D(Size.X, Size.Y);
	case EChartOrigin::RightCenter: return FVector2D(Size.X, Size.Y * 0.5);
	case EChartOrigin::RightTop: return FVector2D(Size.X,0);

	default: return FVector2D(0, Size.Y);
	}
}

int32 SChartAxes::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                          FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                          bool bParentEnabled) const
{
	FVector2D Size = GetOriginPosition(AllottedGeometry.GetLocalSize());
	//UE_LOG(LogTemp, Warning, TEXT("neo---Size_X:%d; Size_Y:%d"), Size.X, Size.Y);


	//X轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(0, Size.Y), Size, FVector2D(AllottedGeometry.GetLocalSize().X, Size.Y)},
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
		{FVector2D(Size.X, 0), Size, FVector2D(Size.X, AllottedGeometry.GetLocalSize().Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);
	
	return LayerId + 2;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
