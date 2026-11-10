// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    NXChartsTypes.h
 * @brief   公共词汇表：Spec / Frame / Renderer 三层共用的枚举。
 *
 * 设计约束：
 *  - 本文件不依赖任何其它图表类型，也不含业务逻辑，只放"大家都要说同一套话"的定义。
 *  - 所有枚举都是 UENUM(BlueprintType)，因为它们是蓝图可见的配置项。
 */

#pragma once

#include "CoreMinimal.h"
#include "NXChartsTypes.generated.h"

/** 系列类型。当前所有类型都按折线绘制，其余值为将来的柱状/饼图预留 */
UENUM(BlueprintType)
enum class EChartSeriesKind : uint8
{
	Line,
	Bar,
	Scatter,
	Area
};

/** 连线方式。Smooth 用单调三次插值（不会过冲），Step 用于状态量/开关信号 */
UENUM(BlueprintType)
enum class EChartCurveType : uint8
{
	Linear,
	Smooth,
	Step
};

/** 坐标轴。双 Y 轴时用 SecondaryY 扩展 */
UENUM(BlueprintType)
enum class EChartAxis : uint8
{
	X,
	Y
};

/** 尺度类型。Band 用于柱状图的分类轴，Time 用于时间序列（当前仅 Linear 生效） */
UENUM(BlueprintType)
enum class EChartScaleType : uint8
{
	Linear,
	Band,
	Log,
	Time
};

/** 轴范围来源 */
UENUM(BlueprintType)
enum class EChartDomainMode : uint8
{
	/** 由可见系列的数据自动推导 */
	Auto,
	/** 使用 ManualDomain */
	Manual
};

/** 刻度标签格式。Auto 会按步长自动决定小数位 */
UENUM(BlueprintType)
enum class EChartTickFormat : uint8
{
	Auto,
	Integer,
	OneDecimal,
	TwoDecimals,
	Percent,
	Scientific
};

/** 文本水平对齐 */
UENUM(BlueprintType)
enum class EChartTextHAlign : uint8
{
	Left,
	Center,
	Right
};

/** 文本垂直对齐 */
UENUM(BlueprintType)
enum class EChartTextVAlign : uint8
{
	Top,
	Middle,
	Bottom
};

/**
 * 绘制分区。
 * 目的是给绘制元素一个稳定的语义层序，避免"谁先画"依赖数组下标这种隐式约定。
 */
UENUM(BlueprintType)
enum class EChartLayer : uint8
{
	Background,
	Grid,
	Axis,
	/** 注解层：阈值线/色带，画在网格之上、曲线之下 */
	Annotation,
	Series,
	Overlay,
	Tooltip
};