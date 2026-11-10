// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartTheme.h
 * @brief   主题数据资产（DataAsset），用于项目级换肤。
 *
 * 当前状态：FChartSpec 通过 TSoftObjectPtr 引用它，但实际配色取自
 * FChartSpec 里的各个颜色字段——这个类目前是"预留入口"，尚未参与渲染。
 * 若要真正启用，应在布局阶段解析字体/颜色时"先读 Theme、再被 Spec 覆盖"。
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NXChartTheme.generated.h"

UCLASS()
class NXCHARTS_API UNXChartTheme : public UDataAsset
{
	GENERATED_BODY()
public:
	// 系列默认颜色序列（按系列顺序取用）
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
