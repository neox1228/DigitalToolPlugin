// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChartTheme.generated.h"

/**
 * 
 */
UCLASS()
class NXCHARTS_API UChartTheme : public UDataAsset
{
	GENERATED_BODY()
public:
	// 系列默认颜色序列
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FLinearColor> SeriesColors = {
		FLinearColor(0.2f, 0.6f, 1.0f),   // 蓝
		FLinearColor(0.8f, 0.3f, 0.3f),   // 红
		FLinearColor(0.3f, 0.8f, 0.3f),   // 绿
		FLinearColor(0.9f, 0.7f, 0.1f),   // 黄
		FLinearColor(0.6f, 0.2f, 0.8f),   // 紫
	};

	// 坐标轴线颜色
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor AxisLineColor = FLinearColor(0.6f, 0.6f, 0.6f);

	// 网格线颜色
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor GridColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);

	// 标题字体大小
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TitleFontSize = 18;

	// 轴标签字体大小
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AxisLabelFontSize = 12;
	
};
