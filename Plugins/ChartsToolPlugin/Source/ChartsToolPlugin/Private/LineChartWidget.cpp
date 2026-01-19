// Fill out your copyright notice in the Description page of Project Settings.


#include "LineChartWidget.h"

#include "SLineChartWidget.h"
#include "Components/CanvasPanelSlot.h"

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{	
	// 坐标轴Slate创建及初始化
	SlateAxes = SNew(SChartAxes)
	.AxisLayout(AxisLayout)
	.AxisType(AxisType)
	.TickFont(TickFont)
	.AxisColor(AxisColor)
	.AxisThickness(AxisThinkness)
	.GridDensity(GridDensity)
	.ShowGrid(GridToggle);
	
	
	TSharedRef<SOverlay> LineChart = SNew(SOverlay)
	+ SOverlay::Slot()
	.Padding(130.f,160.f, 80.f, 100.f)
	[
		SlateAxes.ToSharedRef()
	];

	if (SeriesMap.Num() > 0)
	{
		for (auto& Elem : SeriesMap)
		{
			const FString& SeriesName = Elem.Key;
			const FSeriesSetting& SeriesSetting = Elem.Value;

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
	
	return LineChart;
}

void ULineChartWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	// 1. 获取当前二维数据中的最大最小值
	FChartMath::GetArrayRange(SeriesMap, AxisLayout.AxisX_Max, AxisLayout.AxisY_Max, EValue::Max);
	FChartMath::GetArrayRange(SeriesMap, AxisLayout.AxisX_Min, AxisLayout.AxisY_Min, EValue::Min);

	// 2. 计算当前坐标系基础数据
	FChartMath::CalculateAsymmetricAxisLayout(AxisLayout.AxisX_Max, AxisLayout.AxisX_Min,
		AxisLayout.AxisX_OutPositiveTicks, AxisLayout.AxisX_OutNegativeTicks, AxisLayout.AxisX_TickStep);
	FChartMath::CalculateAsymmetricAxisLayout(AxisLayout.AxisY_Max, AxisLayout.AxisY_Min,
		AxisLayout.AxisY_OutPositiveTicks, AxisLayout.AxisY_OutNegativeTicks, AxisLayout.AxisY_TickStep);
	UE_LOG(LogTemp, Warning, TEXT("neo---X轴最大值：%f ; X轴最小值：%f ; Step：%f"), AxisLayout.AxisX_Max, AxisLayout.AxisX_Min, AxisLayout.AxisX_TickStep);
	
	// 3. 同步数据
	if (SlateAxes.IsValid())
	{
		SlateAxes->SyncAxisLayout(AxisLayout);
		SlateAxes->SyncGridToggle(GridToggle);
		SlateAxes->SyncGridDensity(GridDensity);
		
		
	}
	for (auto& TempLine : LineMap)
	{
		if (TempLine.Value.IsValid())
		{
			TempLine.Value->SetData(SeriesMap.Find(TempLine.Key)->Data);
			TempLine.Value->SyncAxisLayout(AxisLayout);

			float TempBoundX;
			float TempBoundY;
			FChartMath::GetArrayRange(SeriesMap, TempBoundX, TempBoundY);
			
		}
	}
}

void ULineChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	/*SlateWidget.Reset();*/
	for (auto& Elem : LineMap)
	{
		Elem.Value.Reset();
	}
	SlateAxes.Reset();
}
