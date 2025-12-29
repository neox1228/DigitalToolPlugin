// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FChartMath.h"
#include "Widgets/SCompoundWidget.h"


class CHARTSTOOLPLUGIN_API SChartAxes : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SChartAxes)
	: _Origin(EChartOrigin::LeftBottom)
	,_AxisColor(FLinearColor::White)
	,_AxisThickness(1.5f)
	,_XAxisTicks(5)
	,_YAxisTicks(5)
	,_TickFontSize(10)
	,_ShowGrid(true)
	{}
	
	/*坐标轴原点位置（例如：左上、中心、右下等）*/
	SLATE_ATTRIBUTE(EChartOrigin, Origin)
	
	/*坐标轴风格相关参数 */
	SLATE_ATTRIBUTE(FLinearColor, AxisColor) // 坐标轴颜色
	SLATE_ATTRIBUTE(float, AxisThickness) // 坐标轴粗细

	/*坐标系相关效果设置参数（例如：网格背景，坐标轴刻度等）*/
	SLATE_ATTRIBUTE(bool, ShowGrid) //背景网格开关
	SLATE_ATTRIBUTE(int32, XAxisTicks) // X轴刻度粗细
	SLATE_ATTRIBUTE(int32, YAxisTicks) // Y轴刻度粗细
	SLATE_ATTRIBUTE(int32, TickFontSize) //字体大小
	
	
	
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TAttribute<EChartOrigin> Origin;
	
	TAttribute<FLinearColor> AxisColor;
	TAttribute<float> AxisThickness;

	TAttribute<bool> ShowGrid;
	TAttribute<int32> XAxisTicks;
	TAttribute<int32> YAxisTicks;
	TAttribute<int32> TickFontSize;
	
	
public:

	void SetOrigin(EChartOrigin InOrigin)
	{
		Origin = InOrigin;
	}
	
	virtual int32 OnPaint(const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
};
