// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChartAxes::Construct(const FArguments& InArgs)
{
	Oringin = InArgs._Oringin;
}

FVector2D SChartAxes::GetOringinPosition(const FVector2D& Size)
{
	switch (Oringin)
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
	

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                                bParentEnabled);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
