// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ChartSeriesBuffer.h"
#include "Frame/ChartFrame.h"

/**
 * 帧编译流水线：域 → 刻度 → 布局 → 几何。
 *
 * 这是「数据变化」与「绘制」之间唯一的桥。上层（Widget）只关心"什么时候重建"，
 * 这里关心"怎么重建"，并且区分两个档位：
 *
 *  - Build()          整帧重建：域 / 刻度 / 布局 / 全部几何（Spec 或尺寸变化时）
 *  - ReconcileData()  增量协调：域没变时只重建数据版本变化的那几条系列几何
 *                     （布局与文字测量是整个流程里最贵的部分，能跳过就跳过）
 */
class NXCHARTS_API FChartFrameBuilder
{
public:
	/**
	 * 解析某根轴的数据域。
	 * 优先级：运行时 Override > AxisSpec.ManualDomain > 自动推导
	 */
	static FChartDomain ResolveDomain(EChartAxis Axis, const FChartAxisSpec& AxisSpec,
	                                  const FChartSpec& Spec, const FChartSeriesDataPool& Data,
	                                  const FChartDomainOverride& Override);

	/**
	 * 构建一帧。尺寸过小或域无法解析时返回 false，OutFrame.bValid 保持 false。
	 * 调用方（UNXChartWidget）负责判断是否需要重建。
	 */
	static bool Build(const FChartSpec& Spec, const FChartSeriesDataPool& Data,
	                  const FChartDomainOverride& Override, const FVector2D& WidgetSize,
	                  FChartFrame& OutFrame, const FChartMeasureFunc& Measure = FChartMeasureFunc());

	/**
	 * 数据变更后的协调：
	 *  - 域没变 → 只重建 Revision 变化的系列几何，其余原样复用；
	 *  - 域变了（自动范围随新数据扩张） → 退化为整帧重建，因为刻度与布局都会变。
	 *
	 * 返回 true 表示帧仍然有效（可能发生了整帧重建）。
	 */
	static bool ReconcileData(FChartFrame& InOutFrame, const FChartSpec& Spec,
	                          const FChartSeriesDataPool& Data, const FChartDomainOverride& Override,
	                          const FVector2D& WidgetSize, const FChartMeasureFunc& Measure = FChartMeasureFunc());
};
