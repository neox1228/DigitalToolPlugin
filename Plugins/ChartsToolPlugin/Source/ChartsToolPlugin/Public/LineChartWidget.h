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
	/*//折线图数据（用于测试）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	TArray<FVector2D> Data;*/

	//折线数据 Map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart|Series")
	TMap<FString, FSeriesSetting> SeriesMap;

	TMap<FString, TSharedPtr<SLineChartWidget>> LineMap;

	/**手动坐标轴边界配置参数*/
	UPROPERTY(EditAnywhere, Category="LineChart", meta=(InlineEditConditionToggle))
	bool bIsUseCustomBoundX = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsUseCustomBoundX"), Category="LineChart")
	float CustomBoundX = 100;
	UPROPERTY(EditAnywhere, Category="LineChart", meta=(InlineEditConditionToggle))
	bool bIsUseCustomBoundY = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsUseCustomBoundY"), Category="LineChart")
	float CustomBoundY = 100;
	

	//坐标原点位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	EChartOrigin ChartOrigin = EChartOrigin::LeftBottom;

	//坐标轴颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	FLinearColor AxisColor;

	// 坐标轴粗细
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	float AxisThinkness;

	// 网格背景开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	bool GridToggle = true;

	// 网格背景密度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	int32 GridDensity = 10;

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

	/**
	 * Data数据Get函数
	 * @return TArray<FVector2D> 返回当前Data数组copy
	 */
	/*UFUNCTION(BlueprintCallable, Category="LineChart")
	TArray<FVector2D> GetData()
	{
		return Data;
	}*/

	/**
	 * Data数据 Set 函数
	 * @param InData 折线数据点位数组
	 */
	/*UFUNCTION(BlueprintCallable, Category="LineChart")
	void SetData(TArray<FVector2D> InData)
	{
		Data = InData;

		// 修改数据后手动调用数据同步逻辑
		SynchronizeProperties();
	}*/

	/*EChartOrigin GetChartOrigin() const
	{
		return ChartOrigin;
	}

	void SetChartOrigin(EChartOrigin ChartOrigin)
	{
		this->ChartOrigin = ChartOrigin;
	}

	FLinearColor GetAxisColor() const
	{
		return AxisColor;
	}

	void SetAxisColor(const FLinearColor& AxisColor)
	{
		this->AxisColor = AxisColor;
	}

	float GetAxisThinkness() const
	{
		return AxisThinkness;
	}

	void SetAxisThinkness(float AxisThinkness)
	{
		this->AxisThinkness = AxisThinkness;
	}

	FLinearColor GetLinearColor() const
	{
		return LinearColor;
	}

	void SetLinearColor(const FLinearColor& LinearColor)
	{
		this->LinearColor = LinearColor;
	}

	float GetLineThickness() const
	{
		return LineThickness;
	}

	void SetLineThickness(float LineThickness)
	{
		this->LineThickness = LineThickness;
	}*/
};
