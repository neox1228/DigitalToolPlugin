// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Feature/Modules/ChartModule.h"
#include "CartesianCoordModule.generated.h"

/**
 * 
 */
UCLASS()
class NXCHARTS_API UCartesianCoordModule : public UChartModule
{
	GENERATED_BODY()
public:
	// 用户配置的轴范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D XAxisRange = FVector2D(0, 100);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D YAxisRange = FVector2D(0, 100);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoAdjustRange = true;

	// 实际绘图使用的缓存范围（可能根据数据自动调整）
	FVector2D CachedXRange, CachedYRange;

	virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry) override;
	virtual void NotifyComponentDataChanged(UChartComponent* Component) override;
	virtual FBox2D GetPlotBounds(const FGeometry& AllottedGeometry) const override;

	// 获取像素映射后的绘图区域（去除内边距）
	FBox2D GetPaddedPlotBounds(const FGeometry& AllottedGeometry) const;
};
