// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ChartComponent.generated.h"

/**
 * 
 */
class UChartModule;
class UChartWidget;
class ChartDrawContext;

UCLASS(Abstract, Blueprintable, EditInlineNew)
class NXCHARTS_API UChartComponent : public UObject
{
	GENERATED_BODY()
	
public:
	// 组件名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ComponentName;

	// 所属模块名称，在图表初始化时查找
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ParentModuleName;

	// 可见性
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisible = true;

	// 收集绘制元素
	virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry) {}

	// 命中测试：给定局部坐标，返回命中的数据项索引，-1 为未命中
	virtual int32 HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const { return -1; }

	// 当绑定的模块广播 OnModuleUpdated 时调用，用于更新内部缓存
	virtual void OnModuleChanged() {}

	// 内部辅助：通知父模块数据已变更
	void NotifyDataChanged();

	// 获取图表控件（遍历 Outer）
	UChartWidget* GetChartWidget() const;

	// 获取绑定的模块
	UChartModule* GetParentModule() const;

	// 蓝图可绑定的交互事件
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentClicked, UChartComponent*, Component, int32, ItemIndex);
	UPROPERTY(BlueprintAssignable)
	FOnComponentClicked OnComponentClicked;
};
