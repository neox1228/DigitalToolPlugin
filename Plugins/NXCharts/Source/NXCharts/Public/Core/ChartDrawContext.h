// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rendering/DrawElements.h"

/**
 * 一个纯 C++ 封装了 Slate 底层绘制的三个核心对象：
 *	FSlateWindowElementList(绘制元素列表)
 *	FGeometry(当前控件的几何信息)
 *	LayerId(当前绘制层级)
 */ 
class NXCHARTS_API ChartDrawContext
{
public:
	
	// 构造函数
	ChartDrawContext(FSlateWindowElementList& InOutDrawElements, const FGeometry& InAllotedGeometry, int32 InStartLayer)
		:OutDrawElements(InOutDrawElements)
		,AllottedGeometry(InAllotedGeometry)
		,CurrentLayer(InStartLayer)
	{}
	
	// 获取下一层 ID 并递增
	int32 ReserveLayer(){return CurrentLayer++; }
	
	//绘制线段
	void DrawLine(const TArray<FVector2D>& Points, const FLinearColor& Color, float Thickness = 1.0f, bool bAntialias = true)
	{
		if (Points.Num() < 2) return;
		TArray<FVector2f> FloatPoints;
		FloatPoints.Reserve(Points.Num());
		for (const FVector2D& Point : Points) FloatPoints.Add(FVector2f(Point));
		
		FSlateDrawElement::MakeLines(OutDrawElements, ReserveLayer()
			, AllottedGeometry.ToPaintGeometry()
			, FloatPoints, ESlateDrawEffect::None
			, Color, bAntialias, Thickness);
	}
	
	// 绘制矩形
	void DrawRect(const FVector2D& TopLeft, const FVector2D& Size, const FLinearColor& Color)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, ReserveLayer(),
			AllottedGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(TopLeft)),
			FCoreStyle::Get().GetDefaultBrush(), ESlateDrawEffect::None, Color);
	}
	
	// 绘制文本（）
	~ChartDrawContext() {}
private:
	FSlateWindowElementList& OutDrawElements;
	const FGeometry& AllottedGeometry;
	int32 CurrentLayer;

};
