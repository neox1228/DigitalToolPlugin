// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartLayout.h
 * @brief   布局：域 + 刻度 + 文字实测 -> 绘图区、图例区、轴标题区。
 *
 * 为什么必须有这一层：绘图区不能拍常量内边距。Y 轴最宽标签可能是 "1500"
 * 也可能是 "1200000"，必须先测出文字尺寸才能决定左侧留多宽。
 *
 * 图例条目矩形也在这里产出：绘制与点击命中共用同一份，避免两处各算一遍产生漂移。
 */

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/ChartSpace.h"
#include "Layout/ChartTicks.h"
#include "Spec/NXChartSpec.h"

/** 单个图例条目：既用于绘制，也用于点击命中 */
struct NXCHARTS_API FChartLegendItem
{
	FName SeriesKey;

	/** 整个条目的可点击区域（局部坐标） */
	FBox2D Rect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));

	/** 色块区域 */
	FBox2D MarkerRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));
};

/** 单根轴的布局结果：刻度、标签实测尺寸、标题、为它预留的边距 */
struct NXCHARTS_API FChartAxisLayout
{
	FChartTickSet TickSet;

	/** 所有主刻度标签里的最大尺寸（Y 轴用它决定左侧宽度） */
	FVector2D MaxLabelSize = FVector2D::ZeroVector;

	FText Title;
	FVector2D TitleSize = FVector2D::ZeroVector;

	/** Y 轴：左侧预留宽度；X 轴：下方预留高度 */
	double ReservedSize = 0.0;
};

/** 一帧的布局结果 */
struct NXCHARTS_API FChartLayout
{
	FBox2D PlotRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));
	FBox2D TitleRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));
	FBox2D LegendRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));

	/** 轴标题各自的绘制区（Y 轴标题横向排在绘图区上方，不做旋转） */
	FBox2D XAxisTitleRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));
	FBox2D YAxisTitleRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));

	FChartAxisLayout XAxis;
	FChartAxisLayout YAxis;

	/** 图例条目（含每项矩形）。空表示不显示图例 */
	TArray<FChartLegendItem> LegendItems;

	FVector2D WidgetSize = FVector2D::ZeroVector;
	EChartLegendPosition LegendPosition = EChartLegendPosition::Bottom;

	/** 解析后的字体：渲染必须用这一份，保证"测量"和"绘制"用的是同一个字体 */
	FSlateFontInfo AxisLabelFont;
	FSlateFontInfo TitleFont;
	FSlateFontInfo LegendFont;

	bool bValid = false;
	bool bHasLegend = false;
};

/** 文字测量回调。默认实现走 Slate 的字体度量服务；单测可以注入假实现 */
using FChartMeasureFunc = TFunction<FVector2D(const FString& Text, const FSlateFontInfo& Font)>;

class NXCHARTS_API FChartLayoutBuilder
{
public:
	/**
	 * 计算布局。WidgetSize 为控件局部尺寸，XDomain / YDomain 为本帧解析出的数据域。
	 * 尺寸太小或域无效时返回 bValid=false 的结果，不抛异常。
	 */
	static FChartLayout Build(const FChartSpec& Spec,
	                          const FChartDomain& XDomain,
	                          const FChartDomain& YDomain,
	                          const FVector2D& WidgetSize,
	                          const FChartMeasureFunc& Measure = FChartMeasureFunc());

	/** 轴标题 = Title + (Unit)，Unit 为空则直接用 Title */
	static FText ComposeAxisTitle(const FChartAxisSpec& AxisSpec);
};