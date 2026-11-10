// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartFrame.h
 * @brief   一帧的不可变快照（POD 语义，不持有 UObject）。
 *
 * 绘制期只读这个对象：不布局、不映射坐标、不 NewObject、不碰 UObject 属性。
 * 帧由 FChartFrameBuilder 产出，只在「配置版本 / 数据版本 / 控件尺寸」变化时重建。
 */

#pragma once

#include "CoreMinimal.h"
#include "Core/NXChartsTypes.h"
#include "Layout/ChartLayout.h"
#include "Layout/ChartSpace.h"
#include "Spec/NXChartSpec.h"

/**
 * 一条系列在本帧的几何结果：已经映射到控件局部坐标，绘制时不需要再做任何变换。
 * 这里用 FVector2d 而不是 FVector2f，是为了直接喂给 MakeLines 的 double 重载，避免每帧转换。
 */
struct NXCHARTS_API FChartSeriesGeometry
{
	FName SeriesKey;
	EChartSeriesKind Kind = EChartSeriesKind::Line;
	FLinearColor Color = FLinearColor::White;
	float LineThickness = 2.0f;
	bool bAntialias = true;
	bool bShowMarkers = false;
	float MarkerSize = 3.0f;

	/** 面积填充与发光（由 Style 带过来，绘制时不再回查 Spec） */
	bool bFillArea = false;
	float AreaFillOpacity = 0.45f;
	bool bGlow = false;
	float GlowThickness = 8.0f;

	/** 已映射的局部坐标顶点（可能是抽稀/曲线细分之后的） */
	TArray<FVector2d> LocalPoints;

	/** 这份几何对应的数据版本，用于判断能否复用 */
	int32 DataRevision = INDEX_NONE;
};

/**
 * 一帧的完整快照。
 * SChartCanvas::OnPaint 只读它；重建由 UNXChartWidget::GetFrame 按版本判断。
 */
/** 查询辅助：鼠标悬停与外部十字准星共用同一套「最近点」逻辑，避免两处算法跑偏 */
namespace ChartQuery
{
	/** 在已映射的局部坐标里找 X 最近的点；空数组返回 INDEX_NONE */
	inline int32 FindNearestPointIndex(const TArray<FVector2d>& LocalPoints, double LocalX)
	{
		int32 BestIndex = INDEX_NONE;
		double BestDistance = TNumericLimits<double>::Max();

		for (int32 Index = 0; Index < LocalPoints.Num(); ++Index)
		{
			const double Distance = FMath::Abs(LocalPoints[Index].X - LocalX);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestIndex = Index;
			}
		}

		return BestIndex;
	}
}
struct NXCHARTS_API FChartFrame
{
	FChartSpec Spec;
	FChartLayout Layout;
	FChartCartesianSpace Space;
	TArray<FChartSeriesGeometry> Series;

	/** 构建时所有系列的 Revision 之和 */
	int64 DataRevisionSum = -1;

	/** 构建时的控件尺寸 */
	FVector2D BuildSize = FVector2D::ZeroVector;

	bool bValid = false;

	void Reset()
	{
		Spec = FChartSpec();
		Layout = FChartLayout();
		Space = FChartCartesianSpace();
		Series.Reset();
		DataRevisionSum = -1;
		BuildSize = FVector2D::ZeroVector;
		bValid = false;
	}

	const FChartSeriesGeometry* FindSeries(FName Key) const
	{
		for (const FChartSeriesGeometry& Item : Series)
		{
			if (Item.SeriesKey == Key) return &Item;
		}
		return nullptr;
	}
};