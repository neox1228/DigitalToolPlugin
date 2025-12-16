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
	: _Oringin(EChartOrigin::LeftBottom)
	,_AxisColor(FLinearColor::White)
	,_AxisThickness(1.5f)
	,_XAxisMin(-1000.f)
	,_XAxisMax(1000.f)
	,_XAxisTicks(5)
	,_YAxisMin(-1000.f)
	,_YAxisMax(1000.f)
	,_YAxisTicks(5)
	,_TickFontSize(10)
	,_ShowGrid(true)
	,_SlotSize(FVector2D::ZeroVector)
	{}
	
	/*
	 *坐标轴相关属性：
	 * 1. 坐标轴位置，（八方向）
	 * 2. 坐标轴颜色
	 * 3. 坐标轴粗细
	 */
	SLATE_ATTRIBUTE(EChartOrigin, Oringin)
	SLATE_ATTRIBUTE(FLinearColor, AxisColor)
	SLATE_ATTRIBUTE(float, AxisThickness)

	/*
	 *坐标轴范围和刻度：
	 */
	SLATE_ATTRIBUTE(float, XAxisMin) // X轴最小值
	SLATE_ATTRIBUTE(float, XAxisMax) // X轴最大值
	SLATE_ATTRIBUTE(int32, XAxisTicks) // X轴刻度
	SLATE_ATTRIBUTE(float, YAxisMin) // Y轴最小值
	SLATE_ATTRIBUTE(float, YAxisMax) // Y轴最大值
	SLATE_ATTRIBUTE(int32, YAxisTicks) // Y轴刻度
	SLATE_ATTRIBUTE(FVector2D, SlotSize)// 绘制框大小

	/*
	 *坐标轴效果
	 */
	SLATE_ATTRIBUTE(int32, TickFontSize)//字体大小
	SLATE_ATTRIBUTE(bool, ShowGrid)//背景网格开关
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TAttribute<EChartOrigin> Oringin;
	TAttribute<FLinearColor> AxisColor;
	TAttribute<float> AxisThickness;

	TAttribute<float> XAxisMin;
	TAttribute<float> XAxisMax;
	TAttribute<int32> XAxisTicks;
	TAttribute<float> YAxisMin;
	TAttribute<float> YAxisMax;
	TAttribute<int32> YAxisTicks;
	TAttribute<FVector2D> SlotSize;

	TAttribute<int32> TickFontSize;
	TAttribute<bool> ShowGrid;

	mutable  FGeometry  CacheGeometry;

protected:
	
	
public:
	FVector2D GetOriginPosition(const FVector2D& Size) const;
	// 将 点位坐标映射到 widegt 本地坐标，用于绘制折线等
	FVector2D DataToLocal( const FVector2D& DataPoint) const;

	// 获取当前 X轴范围
	void GetXRange(float& OutMin, float& OutMax) const;

	// 获取当前 Y轴范围
	void GetYRange(float& OutMin, float& OutMax) const;

	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
