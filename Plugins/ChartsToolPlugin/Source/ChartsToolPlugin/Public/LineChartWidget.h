// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FChartMath.h"
#include "SChartAxes.h"
#include "SLineChartWidget.h"
#include "Components/Widget.h"
#include "LineChartWidget.generated.h"


/** 前向声明 */

/**
 * 
 */
UCLASS()
class CHARTSTOOLPLUGIN_API ULineChartWidget : public UWidget
{
	GENERATED_BODY()

public:

	/** 坐标系相关参数 **/
	// 坐标轴模式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|AxisSetting")
	EAxisType AxisType = EAxisType::Numeric;
	//坐标系颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|AxisSetting")
	FLinearColor AxisColor;
	// 坐标系粗细
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|AxisSetting")
	float AxisThinkness;
	// 坐标系刻度值字体
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|AxisSetting")
	FSlateFontInfo TickFont = FCoreStyle::Get().GetFontStyle("NormalFont");
	// 网格背景开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|AxisSetting")
	bool GridToggle = true;
	// 网格背景密度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|AxisSetting")
	int32 GridDensity = 10;

	/** 折线相关参数 **/
	//折线数据 Map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|Series")
	TMap<FString, FSeriesSetting> SeriesMap;

protected:

	// 坐标系Slate
	TSharedPtr<SChartAxes> SlateAxes;

	// 折线Slate Map
	TMap<FString, TSharedPtr<SLineChartWidget>> LineMap;

	// AxisLayout
	UPROPERTY()
	FAxisLayout AxisLayout;


public:
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	
};
