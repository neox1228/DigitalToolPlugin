// Fill out your copyright notice in the Description page of Project Settings.


#include "LineChartWidget.h"

#include "SLineChartWidget.h"
#include "Components/CanvasPanelSlot.h"

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{	
	// 坐标轴Slate创建及初始化
	if (!SlateAxes.IsValid())
	{
		SlateAxes = SNew(SChartAxes)
		.AxisLayout(AxisLayout)
		.AxisType(AxisType)
		.TickFont(TickFont)
		.AxisColor(AxisColor)
		.AxisThickness(AxisThinkness)
		.GridDensity(GridDensity)
		.ShowGrid(GridToggle);
	}
	LineChart = SNew(SOverlay)
	+ SOverlay::Slot()
	.Padding(130.f,160.f,80.f,100.f)
	[
		SlateAxes.ToSharedRef()
	];

	UpdataLineMap();
	return LineChart.ToSharedRef();
}

void ULineChartWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	//UE_LOG(LogTemp, Warning, TEXT("neo---Widget大小 X: %f, Y: %f"),SlateAxes->GetCachedGeometry().GetLocalSize().X, SlateAxes->GetCachedGeometry().GetLocalSize().Y);
	// 1. 获取当前二维数据中的最大最小值
	FChartMath::GetArrayRange(SeriesMap, AxisLayout.AxisX_Max, AxisLayout.AxisY_Max, EValue::Max);
	FChartMath::GetArrayRange(SeriesMap, AxisLayout.AxisX_Min, AxisLayout.AxisY_Min, EValue::Min);

	// 2. 计算当前坐标系基础数据
	FChartMath::CalculateAsymmetricAxisLayout(AxisLayout.AxisX_Max, AxisLayout.AxisX_Min,
		AxisLayout.AxisX_OutPositiveTicks, AxisLayout.AxisX_OutNegativeTicks, AxisLayout.AxisX_TickStep);
	FChartMath::CalculateAsymmetricAxisLayout(AxisLayout.AxisY_Max, AxisLayout.AxisY_Min,
		AxisLayout.AxisY_OutPositiveTicks, AxisLayout.AxisY_OutNegativeTicks, AxisLayout.AxisY_TickStep);
	//UE_LOG(LogTemp, Warning, TEXT("neo---X轴最大值：%d ; X轴最小值：%d ; Step：%f"), AxisLayout.AxisX_OutPositiveTicks,AxisLayout.AxisX_OutNegativeTicks, AxisLayout.AxisX_TickStep);
	//UE_LOG(LogTemp, Warning, TEXT("neo---Y轴最大值：%d ; Y轴最小值：%d ; Step：%f"), AxisLayout.AxisY_OutPositiveTicks,AxisLayout.AxisY_OutNegativeTicks, AxisLayout.AxisY_TickStep);
	
	// 3. 同步数据
	if (SlateAxes.IsValid())
	{
		SlateAxes->SyncAxisLayout(AxisLayout);
		SlateAxes->SyncGridToggle(GridToggle);
		SlateAxes->SyncGridDensity(GridDensity);
	}
	UpdataLineMap();
}

void ULineChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	
	for (auto& Elem : LineMap)
	{
		Elem.Value.Reset();
	}
	SlateAxes.Reset();
	LineChart.Reset();
}
