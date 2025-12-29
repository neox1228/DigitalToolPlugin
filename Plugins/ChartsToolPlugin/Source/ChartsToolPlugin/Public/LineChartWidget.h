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
	//折线图数据（用于测试）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	TArray<FVector2D> Data;
	
	//坐标原点位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	EChartOrigin ChartOrigin = EChartOrigin::LeftBottom;

	//坐标轴颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	FLinearColor AxisColor;

	// 坐标轴粗细
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	float AxisThinkness;

	// 折线颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	FLinearColor LinearColor = FLinearColor::White;

	//折线粗细
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	float LineThickness = 2.f;


protected:

	TSharedPtr<SLineChartWidget> SlateWidget;
	TSharedPtr<SChartAxes> SlateAxes;

	FVector2D SlotSize = FVector2D::ZeroVector;
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION(BlueprintCallable, Category="LineChart")
	TArray<FVector2D> GetData()
	{
		return Data;
	}

	
	UFUNCTION(BlueprintCallable, Category="LineChart")
	void SetData(TArray<FVector2D> InData)
	{
		Data = InData;
	}
};
