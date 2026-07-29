// Fill out your copyright notice in the Description page of Project Settings.


#include "Feature/Components/ChartComponent.h"
#include "Feature/Modules/ChartModule.h"
#include "Widget/ChartWidget.h"

void UChartComponent::NotifyDataChanged()
{
	if (UChartModule* Mod = GetParentModule())
	{
		Mod->NotifyComponentDataChanged(this);
	}
}

UChartWidget* UChartComponent::GetChartWidget() const
{
	// 向上遍历 Outer 查找 UChartWidget
	for (UObject* OuterObj = GetOuter(); OuterObj; OuterObj = OuterObj->GetOuter())
	{
		if (UChartWidget* Widget = Cast<UChartWidget>(OuterObj))
		{
			return Widget;
		}
	}
	return nullptr;
}

UChartModule* UChartComponent::GetParentModule() const
{
	UChartWidget* Widget = GetChartWidget();
	if (!Widget) return nullptr;
	for (UChartModule* Mod : Widget->Modules)
	{
		if (Mod && Mod->ModuleName == ParentModuleName)
		{
			return Mod;
		}
	}
	return nullptr;
}
