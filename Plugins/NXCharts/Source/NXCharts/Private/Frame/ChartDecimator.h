// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 折线抽稀。
 *
 * 问题：`MakeLines` 的每帧成本与顶点数成正比，10 万点直接把 Slate 拖死。
 * 做法：按「像素列」分桶，每列只保留 Y 最小与最大的两个真实数据点。
 * 结果：顶点数上界从 O(数据量) 变成 O(绘图区宽度)，而包络形状不失真
 *（这就是金融/监控图表里常见的 min-max decimation）。
 *
 * 注意：保留的一定是**真实数据点**（不是插值），所以 tooltip 取到的是真值。
 */
namespace ChartDecimate
{
	/**
	 * @param Points          数据点（数据坐标，X 应大致升序）
	 * @param XMin/XMax       当前 X 轴数据域
	 * @param PlotWidth       绘图区像素宽度（决定列数）
	 * @param MaxOutputPoints 数据量不超过它时直接返回全部索引，不做抽稀
	 * @param OutIndices      升序保留的顶点索引
	 */
	NXCHARTS_API void MinMaxByColumn(const TArray<FVector2d>& Points, double XMin, double XMax,
	                                 double PlotWidth, int32 MaxOutputPoints, TArray<int32>& OutIndices);
}
