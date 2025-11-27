// Fill out your copyright notice in the Description page of Project Settings.


#include "LineChartWidget.h"

#include "SLineChartWidget.h"

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{
	SlateWidget = SNew(SLineChartWidget)
	.Data(BIND_UOBJECT_ATTRIBUTE(TArray<FVector2D>, GetData))
	.LineColor(LinearColor)
	.LineThinckness(LineThickness);

	SlateAxes = SNew(SChartAxes)
	.Oringin(ChartOrigin)
	.AxisColor(AxisColor)
	.AxisThickness(AxisThinkness);
	
	
	TSharedRef<SOverlay> LineChart = SNew(SOverlay)

	+SOverlay::Slot()
	[
		SlateAxes.ToSharedRef()
	]
	+ SOverlay::Slot()
	[
		SlateWidget.ToSharedRef()
	];
	
	return LineChart;
}

void ULineChartWidget::SyncProperties()
{
	Super::SynchronizeProperties();
	if (SlateWidget.IsValid())
	{
		SlateWidget->SetData(Data);
	}
}

void ULineChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlateWidget.Reset();
}
