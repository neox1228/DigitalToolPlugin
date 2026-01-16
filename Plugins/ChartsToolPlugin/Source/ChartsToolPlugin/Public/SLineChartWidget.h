// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FChartMath.h"
#include "Widgets/SCompoundWidget.h"

/** 前向声明 */

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
	    SLATE_ATTRIBUTE(FVector2D, CurrentSize)
		SLATE_ATTRIBUTE(float, CustomBoundX)
		SLATE_ATTRIBUTE(float, CustomBoundY)
		SLATE_ATTRIBUTE(float, MaxX)
		SLATE_ATTRIBUTE(float, MaxY)
		SLATE_ATTRIBUTE(float, MinX)
		SLATE_ATTRIBUTE(float, MinY)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TAttribute<float> CustomBoundX;
	TAttribute<float> CustomBoundY;
	TAttribute<float> MaxX;
	TAttribute<float> MaxY;
	TAttribute<float> MinX;
	TAttribute<float> MinY;
	TAttribute<EChartOrigin> Origin;
	TAttribute<TArray<FVector2D>> Data;
	TAttribute<FLinearColor> LineColor;
	TAttribute<float> LineThinckness;
	

public:
	void SetOrigin(EChartOrigin InOrigin)
	{
		Origin = InOrigin;
	}
	void SetData(TArray<FVector2D> InData)
	{
		Data = InData;
	}

	void SyncCustomBoundProperty(float InCustomBoundX, float InCustomBoundY)
	{
		
		CustomBoundX = InCustomBoundX;
		CustomBoundY = InCustomBoundY;
	}

	void SyncDataBound(float InMaxX, float InMaxY, float InMinX, float InMinY)
	{
		MaxX = InMaxX;
		MaxY = InMaxY;
		MinX = InMinX;
		MinY = InMinY;
	}
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
