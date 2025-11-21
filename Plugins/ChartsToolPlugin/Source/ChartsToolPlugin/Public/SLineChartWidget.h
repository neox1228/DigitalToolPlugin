// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class CHARTSTOOLPLUGIN_API SLineChartWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLineChartWidget)
	{}
		SLATE_ATTRIBUTE(TArray<FVector2D>, Data)
		SLATE_ATTRIBUTE(FLinearColor, LineColor)
		SLATE_ATTRIBUTE(float, LineThinckness)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TAttribute<TArray<FVector2D>> Data;
	TAttribute<FLinearColor> LineColor;
	TAttribute<float> LineThinckness;

public:
	void SetData(TArray<FVector2D> InData)
	{
		Data = InData;
	}
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
