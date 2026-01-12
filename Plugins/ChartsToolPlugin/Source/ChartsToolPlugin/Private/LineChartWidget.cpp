// Fill out your copyright notice in the Description page of Project Settings.


#include "LineChartWidget.h"

#include "SLineChartWidget.h"
#include "Components/CanvasPanelSlot.h"

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{
	
	
	/*SlateWidget = SNew(SLineChartWidget)
	.Data(BIND_UOBJECT_ATTRIBUTE(TArray<FVector2D>, GetData))
	.LineColor(LinearColor)
	.LineThinckness(LineThickness);*/

	SlateAxes = SNew(SChartAxes)
	.AxisColor(AxisColor)
	.AxisThickness(AxisThinkness);
	
	
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
			.LineThinckness(LineThickness);

			LineMap.Add(SeriesName, NewLine);
		
			// 将折线添加到叠层
			LineChart->AddSlot()
			.Padding(130.f,160.f, 80.f, 100.f)
			[
				NewLine
			];
		}
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("neo---OverlaySize( X:%f, Y:%f)"),LineChart->GetDesiredSize().X, LineChart->GetDesiredSize().Y);

	return LineChart;
}

void ULineChartWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (SlateAxes.IsValid())
	{
		SlateAxes->SetOrigin(ChartOrigin);
		SlateAxes->SyncGridToggle(GridToggle);
		SlateAxes->SyncGridDensity(GridDensity);
		SlateAxes->SyncDataBound(CustomBoundX, CustomBoundY);
	}
	for (auto& TempLine : LineMap)
	{
		
		if (TempLine.Value.IsValid())
		{
			TempLine.Value->SetData(SeriesMap.Find(TempLine.Key)->Data);
			TempLine.Value->SetOrigin(ChartOrigin);

			float TempBoundX;
			float TempBoundY;
			FChartMath::GetArrayRange(SeriesMap, TempBoundX, TempBoundY);
			
			if (!bIsUseCustomBoundX)
			{
				CustomBoundX = TempBoundX;
			}
			if (!bIsUseCustomBoundY)
			{
				CustomBoundY = TempBoundY;
			}
			TempLine.Value->SyncCustomBoundProperty(CustomBoundX,CustomBoundY);
		}
		
	}
	/*if (SlateWidget.IsValid())
	{
		SlateWidget->SetData(Data);
		SlateWidget->SetOrigin(ChartOrigin);
		SlateWidget->SyncCustomBoundProperty(bIsUseCustomBoundX, bIsUseCustomBoundY,CustomBoundX,CustomBoundY);
	}*/
	
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
