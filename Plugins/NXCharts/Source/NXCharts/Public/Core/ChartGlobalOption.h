// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChartGlobalOption.generated.h"

/**
 * 用于存放整个图标的外观属性，比如背景色、内边距、是否抗锯齿、默认字体等
 */
USTRUCT(BlueprintType)// UHT(Unreal Header Tool)编译项目时，UNT会自动为该头文件生成一个配套的.generared.h 文件
struct FChartGlobalOption
{
	GENERATED_BODY()
	/*
	 * 就是把 UHT 在后端自动生成的那些复杂代码的引擎底层代码，“植入”到类的定义中。
	 * 1. 注入反射数据：注册类的元数据、属性和函数，让引擎能够感知这个类
	 * 2. 支持蓝图交互：提供反射接口，使蓝图能够识别、继承、调用该C++ 类及其成员
	 * 3. 支持序列化与存档：让引擎知道如何将这个类的对象保存到磁盘，或从磁盘中恢复
	 * 4. 支持垃圾回收：将对象纳入虚幻引擎的 GC 管理系统中，防止内存泄露
	 * 5. 生成样板代码：自动生成如 StaticClass（）、类型转换辅助函数（如 Cast<T>）以及构造函数相关的辅助声明。
	 */

	
	// 图表背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BackgroundColor = FLinearColor::Black;
	
	// 图表内边距 （左、上、右、下）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin Padding = FMargin(40, 20, 20, 30);
	
	// 是否抗锯齿
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAntiAlias = true;
	
	// 全局字体 （可用于标题、轴标签等，暂时用 FSlateFontInfo 但它是非反射类型，这里先用 FName 存字体名称）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DefaultFontName = "Default";
	
	
};
