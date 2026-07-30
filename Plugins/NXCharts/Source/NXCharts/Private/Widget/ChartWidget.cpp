// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/ChartWidget.h"
#include "Core/ChartCanvas.h"
#include "Feature/Modules/ChartModule.h"
#include "Feature/Components/ChartComponent.h"

TSharedRef<SWidget> UChartWidget::RebuildWidget()
{
	ChartCanvas = SNew(SChartCanvas, this);
	return ChartCanvas.ToSharedRef();
}

void UChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	ChartCanvas.Reset();
}

void UChartWidget::RefreshChart()
{
	if (ChartCanvas.IsValid())
	{
		ChartCanvas->Invalidate(EInvalidateWidget::Layout | EInvalidateWidget::Paint);
	}
}

UChartModule* UChartWidget::AddModule(TSubclassOf<UChartModule> ModuleClass)
{
	if (!ModuleClass) return nullptr;
	UChartModule* NewModule = NewObject<UChartModule>(this, ModuleClass);
	Modules.Add(NewModule);
	RefreshChart();
	return NewModule;
}

UChartComponent* UChartWidget::AddComponent(TSubclassOf<UChartComponent> ComponentClass, FName ModuleName)
{
	if (!ComponentClass) return nullptr;
	UChartComponent* NewComp = NewObject<UChartComponent>(this, ComponentClass);
	if (!ModuleName.IsNone())
	{
		NewComp->ParentModuleName = ModuleName;
	}
	Components.Add(NewComp);
	RefreshChart();
	return NewComp;
}

UChartModule* UChartWidget::GetModuleByName(FName ModuleName) const
{
	UChartModule* CurrentModule = nullptr;
	for (UChartModule* Module : Modules)
	{
		if (Module->ModuleName == ModuleName)
		{
			CurrentModule = Module;
			break;
		}
	}
	return CurrentModule;
}

UChartComponent* UChartWidget::GetComponentByName(FName ComponentName) const
{
	UChartComponent* CurrentComponent = nullptr;
	for (UChartComponent* Component : Components)
	{
		CurrentComponent = Component;
		break;
	}
	return CurrentComponent;
}
