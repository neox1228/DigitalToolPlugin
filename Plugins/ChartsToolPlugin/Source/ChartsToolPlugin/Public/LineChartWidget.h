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

	// 根容器
	TSharedPtr<SOverlay> LineChart;

	// 折线Slate Map
	TMap<FString, TSharedPtr<SLineChartWidget>> LineMap;

	// AxisLayout
	UPROPERTY()
	FAxisLayout AxisLayout;


public:
	void UpdataLineMap()
	{
		if (SeriesMap.Num() <= 0)
		{
			if (LineMap.Num() > 0)
			{
				LineMap.Reset();
				LineMap.Empty();
			}
			return;
		}
		TArray<FString> CurrentKeys;
		LineMap.GenerateKeyArray(CurrentKeys);
		for (auto& Key : CurrentKeys)
		{
			if (!SeriesMap.Contains(Key))
			{
				TSharedPtr<SLineChartWidget>* FoundPtr = LineMap.Find(Key);
				if (FoundPtr->IsValid())
				{
					TSharedPtr<SLineChartWidget> LineToRemove = *FoundPtr;
					LineChart->RemoveSlot(LineToRemove.ToSharedRef());
					LineMap.Remove(Key);
				}
			}
		}
		if (LineMap.Num() == 0 || LineMap.Num() > 10)
		{
			LineMap.Shrink();
		}
		
		for (auto& Elem : SeriesMap)
		{
			const FString& SeriesName = Elem.Key;
			const FSeriesSetting& SeriesSetting = Elem.Value;

			if (LineMap.Contains(SeriesName))
			{
				TSharedPtr<SLineChartWidget>* FoundPtr = LineMap.Find(SeriesName);
				if (FoundPtr->IsValid())
				{
					TSharedPtr<SLineChartWidget> TempLine = *FoundPtr;
					if (TempLine.IsValid())
					{
						TempLine->SetData(SeriesSetting.Data);
						TempLine->SyncAxisLayout(AxisLayout);
						TempLine->SyncSeriesSetting(Elem.Value);
						TempLine->IsCanUpdateData(true);
					}
				}
			}
			else
			{
				//创建新的折线 Slate 实例
				TSharedRef<SLineChartWidget> NewLine = SNew(SLineChartWidget)
				.Data(SeriesSetting.Data)
				.LineColor(SeriesSetting.SeriesColor)
				.LineThinckness(SeriesSetting.SeriesThinckness);

				LineMap.Add(SeriesName, NewLine);
		
				// 将折线添加到叠层
				LineChart->AddSlot()
				.Padding(130.f,160.f, 80.f, 100.f)
				[
					NewLine
				];
			}
		}	
		
	}
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	
};
