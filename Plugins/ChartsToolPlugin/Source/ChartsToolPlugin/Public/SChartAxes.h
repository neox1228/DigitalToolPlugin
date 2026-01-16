// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FChartMath.h"
#include "Widgets/SCompoundWidget.h"


class CHARTSTOOLPLUGIN_API SChartAxes : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SChartAxes)
	: _AxisType(EAxisType::Numeric)
	,_AxisColor(FLinearColor::White)
	,_AxisThickness(1.5f)
	,_ShowGrid(true)
	,_GridDensity(10)
	{}

	// AxisLayout
	SLATE_ATTRIBUTE(FAxisLayout, AxisLayout)
	
	// 坐标轴模式
	SLATE_ATTRIBUTE(EAxisType, AxisType)

	// 刻度字体
	SLATE_ATTRIBUTE(FSlateFontInfo, TickFont)
	
	// 坐标轴颜色
	SLATE_ATTRIBUTE(FLinearColor, AxisColor)

	// 坐标轴粗细
	SLATE_ATTRIBUTE(float, AxisThickness) 

	// 背景网格密度
	SLATE_ATTRIBUTE(int32, GridDensity)

	//背景网格开关
	SLATE_ATTRIBUTE(bool, ShowGrid)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:

	TAttribute<FAxisLayout> AxisLayout;
	TAttribute<EAxisType> AxisType;
	TAttribute<FSlateFontInfo> TickFont;
	
	TAttribute<FLinearColor> AxisColor;
	TAttribute<float> AxisThickness;

	TAttribute<int32> GridDensity;
	TAttribute<bool> ShowGrid;
	
	
	
public:

	void SyncAxisLayout(FAxisLayout InAxisLayout)
	{
		AxisLayout = InAxisLayout;
	}
	void SyncGridToggle(bool InShowGrid)
	{
		ShowGrid = InShowGrid;
	}
	
	void SyncGridDensity(int32 InGridDensity)
	{
		GridDensity = InGridDensity;
	}
	
	virtual int32 OnPaint(const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
};
