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
	const TArray<FVector2D>& Points = Data.Get();

	if (Points.Num() < 2)
	{
		return LayerId;
	}

	FVector2D Size = AllottedGeometry.GetLocalSize();

	for (int32 i = 0; i < Points.Num() - 1; i++)
	{
		FVector2D P1 = Points[i];
		FVector2D P2 = Points[i + 1];

		FSlateDrawElement::MakeLines(OutDrawElements,LayerId,AllottedGeometry.ToPaintGeometry(),{P1, P2},ESlateDrawEffect::None, LineColor.Get(),true
			,LineThinckness.Get());
	}
	return LayerId + 1;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
