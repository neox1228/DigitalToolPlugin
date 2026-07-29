// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ChartModule.generated.h"

class UChartComponent;
class ChartDrawContext;

UCLASS(Abstract, Blueprintable, EditInlineNew)
class NXCHARTS_API UChartModule : public UObject
{
	GENERATED_BODY()
public:
	// 模块唯一标识名，用于组件绑定
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ModuleName;
	
	// 收集绘制元素 （由 SChartCanves 调用）
	virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry){}
	
	// 当依赖此模块的组件数据变化时，组件可调用此方法通知模块
	virtual void NotifyComponentDataChanged(UChartComponent* Component){}
	
	// 获取绘图区域 （例如坐标系模块需要提供可用像素范围）
	virtual FBox2D GetPlotBounds(const FGeometry& AllottedGeometry) const {return FBox2D();}
	
	// 蓝图可绑定的事件：当模块状态更新时广播
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModuleUpdated);
	UPROPERTY(BlueprintAssignable)
	FOnModuleUpdated OnModuleUpdated;
	
	// 调用以广播模块更新事件，通知在模块内部状态变化后
	void BroadcastModuleUpdated()
	{
		OnModuleUpdated.Broadcast();
	}
};
