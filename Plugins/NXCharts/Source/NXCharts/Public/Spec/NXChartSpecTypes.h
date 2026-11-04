// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartSpecTypes.h
 * @brief   系列样式/配置、轴配置。
 *
 * 设计要点：SeriesKey 是运行时的唯一句柄（不是数组下标），
 * 所以重排、插队、删除都不会打乱数据绑定；DisplayName 只影响图例文案。
 */

#pragma once

#include "CoreMinimal.h"
#include "Core/NXChartsTypes.h"
#include "NXChartSpecTypes.generated.h"

/** 单条系列的样式 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartSeriesStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor Color = FLinearColor(0.20f, 0.60f, 1.00f, 1.0f);

	/** 线宽（线图/面积图使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float LineThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	bool bAntialias = true;

	/** 是否绘制数据点标记 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	bool bShowMarkers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (EditCondition = "bShowMarkers"))
	float MarkerSize = 3.0f;

	/** 是否出现在图例中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	bool bShowInLegend = true;

	/** 连线方式。Smooth 为单调三次插值，不会在数据点之间过冲 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	EChartCurveType Curve = EChartCurveType::Linear;

	/** 是否在曲线下方做渐变填充（面积图效果） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	bool bFillArea = false;

	/** 填充顶部的透明度，向下渐变到 0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style",
		meta = (EditCondition = "bFillArea", EditConditionHides, ClampMin = "0.0", ClampMax = "1.0"))
	float AreaFillOpacity = 0.45f;

	/** 霓虹感：先画一条粗的低透明度线打底，再画实线 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	bool bGlow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style",
		meta = (EditCondition = "bGlow", EditConditionHides, ClampMin = "1.0"))
	float GlowThickness = 8.0f;
};

/**
 * 一条系列的配置。
 * SeriesKey 是运行时增删改查的唯一句柄——不是数组下标，所以重排/插队不会打乱数据绑定。
 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartSeriesSpec
{
	GENERATED_BODY()

	/** 稳定标识。为空时该系列会被静默跳过（不会报错） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Series")
	FName SeriesKey;

	/** 图例显示名。留空则用 SeriesKey，填了就能写中文/任意文案而不影响绑定 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Series")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Series")
	EChartSeriesKind Kind = EChartSeriesKind::Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Series")
	FChartSeriesStyle Style;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Series")
	bool bVisible = true;

	/** 该系列使用的数值轴。为双 Y 轴预留 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Series")
	EChartAxis ValueAxis = EChartAxis::Y;

	bool IsValid() const { return !SeriesKey.IsNone(); }

	/** 图例上显示的文字：优先 DisplayName，否则回退到 SeriesKey */
	FString GetLegendLabel() const
	{
		return DisplayName.IsEmpty() ? SeriesKey.ToString() : DisplayName.ToString();
	}
};

/** 单根轴的配置 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartAxisSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	FText AxisTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	EChartScaleType Scale = EChartScaleType::Linear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	EChartDomainMode DomainMode = EChartDomainMode::Auto;

	/** 仅 Manual 模式生效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis",
		meta = (EditCondition = "DomainMode == EChartDomainMode::Manual", EditConditionHides))
	FVector2D ManualDomain = FVector2D(0.0, 100.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	EChartTickFormat Format = EChartTickFormat::Auto;

	/** 单位后缀，例如 "rpm"、"C"。非空时拼成轴标题的 "(...)" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	FString Unit;

	/** 追加在每个刻度标签后面，例如 "s" 会得到 1s / 2s / 3s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	FString TickSuffix;

	/** 期望的主刻度数量，实际数量由 nice-number 算法决定 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis", meta = (ClampMin = "2", ClampMax = "20"))
	int32 DesiredTickCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	bool bShowGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axis")
	bool bShowLabels = true;
};