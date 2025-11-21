// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

UENUM(BlueprintType)
enum class EChartOrigin : uint8
{
	LeftBottom     UMETA(DisplayName="Left-Bottom"),
	CenterBottom   UMETA(DisplayName="Center-Bottom"),
	RightBottom    UMETA(DisplayName="Right-Bottom"),

	LeftCenter     UMETA(DisplayName="Left-Center"),
	Center         UMETA(DisplayName="Center"),
	RightCenter    UMETA(DisplayName="Right-Center"),

	LeftTop        UMETA(DisplayName="Left-Top"),
	CenterTop      UMETA(DisplayName="Center-Top"),
	RightTop       UMETA(DisplayName="Right-Top"),
};
/**
 * 
 */
class CHARTSTOOLPLUGIN_API SChartAxes : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SChartAxes)
		{
		}

	SLATE_ATTRIBUTE(EChartOrigin, Oringin)
	SLATE_ATTRIBUTE(FVector2D, OringinPos)
	SLATE_ATTRIBUTE(FVector2D, Size)
	SLATE_ATTRIBUTE(FLinearColor, AxisColor)
	SLATE_ATTRIBUTE(float, AxisThickness)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TAttribute<EChartOrigin> Oringin;
	TAttribute<FVector2D> OringinPos;
	TAttribute<FVector2D> Size;
	TAttribute<FLinearColor> AxisColor;
	TAttribute<float> AxisThickness;

protected:

	void SetOtinginPos(const FVector2D& InOringinPos)
	{
		OringinPos = InOringinPos;
	}
	FVector2D GetOringinPosition(const FVector2D& Size);
	
public:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
