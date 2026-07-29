// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChartComponent.h"
#include "LineSeriesComponent.generated.h"

/**
 * 
 */
UCLASS()
class NXCHARTS_API UlineSeriesComponent : public UChartComponent
{
	GENERATED_BODY()
public:
	//数据点 （世界坐标/数据空间）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> DataPoints;
	
	//样式
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LineColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineWidth = 2.0f;
	
	virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry) override;
	virtual int32 HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const override;
	
private:
	// 转换数据点道像素坐标
	TArray<FVector2D> MapDataToPixel(const FGeometry& AllottedGeometry) const;
};
