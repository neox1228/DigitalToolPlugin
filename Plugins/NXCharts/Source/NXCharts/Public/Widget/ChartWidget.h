// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Core/ChartGlobalOption.h"
#include "ChartWidget.generated.h"

class UChartModule;
class UChartComponent;
class SChartCanvas;
UCLASS()
class NXCHARTS_API UChartWidget : public UWidget
{
	GENERATED_BODY()
public:
	// 全局外观配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartGlobalOption GlobalOption;

	// 模块列表（坐标系、图例等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Chart")
	TArray<UChartModule*> Modules;

	// 组件列表（系列、轴等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Chart")
	TArray<UChartComponent*> Components;

	// 蓝图可调用：刷新图表，触发重绘
	UFUNCTION(BlueprintCallable, Category = "Chart")
	void RefreshChart();

	// 蓝图可调用：动态添加模块
	UFUNCTION(BlueprintCallable, Category = "Chart")
	UChartModule* AddModule(TSubclassOf<UChartModule> ModuleClass);

	// 蓝图可调用：动态添加组件，并可选指定父模块名
	UFUNCTION(BlueprintCallable, Category = "Chart")
	UChartComponent* AddComponent(TSubclassOf<UChartComponent> ComponentClass, FName ModuleName = NAME_None);
	
	// 蓝图可调用：通过名称获取Module
	UFUNCTION(BlueprintCallable, Category = "Chart")
	UChartModule* GetModuleByName(FName ModuleName) const;
	
	// 蓝图可调用：通过名称获取Component
	UFUNCTION(BlueprintCallable, Category = "Chart")
	UChartComponent* GetComponentByName(FName ComponentName) const;

	// 事件：点击数据项
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChartItemClicked, UChartComponent*, Component, int32, ItemIndex);
	UPROPERTY(BlueprintAssignable, Category = "Chart|Events")
	FOnChartItemClicked OnChartItemClicked;

protected:
	// UWidget 重写
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	// Slate 画布实例
	TSharedPtr<SChartCanvas> ChartCanvas;
};
