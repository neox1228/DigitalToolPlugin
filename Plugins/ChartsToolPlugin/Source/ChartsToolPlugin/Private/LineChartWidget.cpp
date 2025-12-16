// Fill out your copyright notice in the Description page of Project Settings.


#include "LineChartWidget.h"

#include "SLineChartWidget.h"
#include "Components/CanvasPanelSlot.h"

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);

	if (CanvasSlot)
	{
		SlotSize = CanvasSlot->GetSize();
	}
	
	SlateWidget = SNew(SLineChartWidget)
	.Data(BIND_UOBJECT_ATTRIBUTE(TArray<FVector2D>, GetData))
	.LineColor(LinearColor)
	.LineThinckness(LineThickness);

	SlateAxes = SNew(SChartAxes)
	.Oringin(ChartOrigin)
	.AxisColor(AxisColor)
	.SlotSize(SlotSize)
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
