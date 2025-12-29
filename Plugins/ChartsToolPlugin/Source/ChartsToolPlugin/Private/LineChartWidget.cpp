// Fill out your copyright notice in the Description page of Project Settings.


#include "LineChartWidget.h"

#include "SLineChartWidget.h"
#include "Components/CanvasPanelSlot.h"

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{
	
	
	SlateWidget = SNew(SLineChartWidget)
	.Data(BIND_UOBJECT_ATTRIBUTE(TArray<FVector2D>, GetData))
	.LineColor(LinearColor)
	.LineThinckness(LineThickness);

	SlateAxes = SNew(SChartAxes)
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
	//UE_LOG(LogTemp, Warning, TEXT("neo---OverlaySize( X:%f, Y:%f)"),LineChart->GetDesiredSize().X, LineChart->GetDesiredSize().Y);

	return LineChart;
}

void ULineChartWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (SlateWidget.IsValid())
	{
		FChartMath::SortByValueX(Data);
		SlateWidget->SetData(Data);
		SlateWidget->SetOrigin(ChartOrigin);
	}
	if (SlateAxes.IsValid())
	{
		SlateAxes->SetOrigin(ChartOrigin);
	}
}

void ULineChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlateWidget.Reset();
}
