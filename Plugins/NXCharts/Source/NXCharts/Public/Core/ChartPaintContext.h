// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartPaintContext.h
 * @brief   绘制收敛层：把 Slate 的绘制细节挡在渲染代码之外。
 *
 * 三条硬约束：
 *  1. 所有坐标都是控件局部坐标（Slate local space）；只有内部做 LocalToAbsolute。
 *  2. PushClip / PopClip 必须成对；深度由本类记账，Pop 过多会被忽略而不是崩溃。
 *  3. 每个图元默认占用一个新 layer（ReserveLayer），层序即调用顺序。
 *
 * 渐变填充是特例：Slate 没有渐变笔刷，DrawGradientBand 用自定义顶点 + 顶点色插值实现，
 * 而 FSlateVertex 的 Position 是窗口空间坐标，所以它内部会自己做一次变换。
 */

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Clipping.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Core/NXChartsTypes.h"

class NXCHARTS_API FChartPaintContext
{
public:

	FChartPaintContext(FSlateWindowElementList& InOutDrawElements, const FGeometry& InAllotedGeometry, int32 InStartLayer)
		: OutDrawElements(InOutDrawElements)
		, AllottedGeometry(InAllotedGeometry)
		, CurrentLayer(InStartLayer)
	{}

	// ---------------------------------------------------------------- 层级

	/** 索取一个新的绘制层并递增 */
	int32 ReserveLayer() { return CurrentLayer++; }

	int32 GetCurrentLayer() const { return CurrentLayer; }

	// ---------------------------------------------------------------- 裁剪

	/** 以控件局部坐标压入一个矩形裁剪区 */
	void PushClip(const FBox2D& LocalRect)
	{
		const FVector2D AbsMin = AllottedGeometry.LocalToAbsolute(LocalRect.Min);
		const FVector2D AbsMax = AllottedGeometry.LocalToAbsolute(LocalRect.Max);
		const FSlateClippingZone Zone(FSlateRect(
			static_cast<float>(AbsMin.X), static_cast<float>(AbsMin.Y),
			static_cast<float>(AbsMax.X), static_cast<float>(AbsMax.Y)));
		OutDrawElements.PushClip(Zone);
		++ClipDepth;
	}

	/** 弹出一层裁剪区；未被压入时什么也不做 */
	void PopClip()
	{
		if (ClipDepth <= 0) return;
		OutDrawElements.PopClip();
		--ClipDepth;
	}

	int32 GetClipDepth() const { return ClipDepth; }

	// ---------------------------------------------------------------- 线

	/** 折线（数据已是局部坐标）。点数小于 2 时直接忽略 */
	void DrawLine(const TArray<FVector2d>& Points, const FLinearColor& Color, float Thickness = 1.0f, bool bAntialias = true)
	{
		if (Points.Num() < 2) return;
		FSlateDrawElement::MakeLines(
			OutDrawElements, static_cast<uint32>(ReserveLayer()),
			AllottedGeometry.ToPaintGeometry(),
			Points, ESlateDrawEffect::None, Color, bAntialias, Thickness);
	}

	/** 两点之间的虚线。每个 dash 是一个独立绘制元素，只用于网格/参考线这种少量场景 */
	void DrawDashedLine(const FVector2D& From, const FVector2D& To, float DashLength, float GapLength,
	                    const FLinearColor& Color, float Thickness = 1.0f)
	{
		const FVector2D Delta = To - From;
		const double Length = Delta.Size();
		if (Length <= KINDA_SMALL_NUMBER) return;
		if (DashLength <= 0.0f || GapLength < 0.0f)
		{
			DrawLine(TArray<FVector2d>({ FVector2d(From), FVector2d(To) }), Color, Thickness, false);
			return;
		}

		const FVector2D Dir = Delta / Length;
		for (double Start = 0.0; Start < Length; Start += (DashLength + GapLength))
		{
			const double End = FMath::Min(Start + DashLength, Length);
			TArray<FVector2d> Segment;
			Segment.Reserve(2);
			Segment.Add(From + Dir * Start);
			Segment.Add(From + Dir * End);
			DrawLine(Segment, Color, Thickness, false);
		}
	}

	/** 十字标记（数据点强调） */
	void DrawCross(const FVector2D& Center, float HalfSize, const FLinearColor& Color, float Thickness = 1.0f)
	{
		TArray<FVector2d> Points;
		Points.Reserve(2);
		Points.Add(FVector2D(Center.X - HalfSize, Center.Y));
		Points.Add(FVector2D(Center.X + HalfSize, Center.Y));
		DrawLine(Points, Color, Thickness, false);

		Points.Reset();
		Points.Add(FVector2D(Center.X, Center.Y - HalfSize));
		Points.Add(FVector2D(Center.X, Center.Y + HalfSize));
		DrawLine(Points, Color, Thickness, false);
	}

	// ---------------------------------------------------------------- 面

	/** 实心矩形 */
	void DrawRect(const FVector2D& TopLeft, const FVector2D& Size, const FLinearColor& Color)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, static_cast<uint32>(ReserveLayer()),
			AllottedGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(FVector2D(TopLeft))),
			FCoreStyle::Get().GetDefaultBrush(), ESlateDrawEffect::None, Color);
	}

	/** 矩形描边 */
	void DrawRectOutline(const FBox2D& Rect, const FLinearColor& Color, float Thickness = 1.0f)
	{
		TArray<FVector2d> Points;
		Points.Reserve(5);
		Points.Add(Rect.Min);
		Points.Add(FVector2D(Rect.Max.X, Rect.Min.Y));
		Points.Add(Rect.Max);
		Points.Add(FVector2D(Rect.Min.X, Rect.Max.Y));
		Points.Add(Rect.Min);
		DrawLine(Points, Color, Thickness, false);
	}

	/** 圆（折线逼近）。用于数据点标记、图例色点 */
	void DrawCircle(const FVector2D& Center, float Radius, const FLinearColor& Color,
	                float Thickness = 1.0f, int32 Segments = 24)
	{
		if (Radius <= 0.0f || Segments < 3) return;
		TArray<FVector2d> Points;
		Points.Reserve(Segments + 1);
		for (int32 Index = 0; Index <= Segments; ++Index)
		{
			const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(Segments);
			Points.Add(FVector2D(Center.X + FMath::Cos(Angle) * Radius, Center.Y + FMath::Sin(Angle) * Radius));
		}
		DrawLine(Points, Color, Thickness, true);
	}

	// ---------------------------------------------------------------- 文字

	/**
	 * 绘制单行文字。Position 是对齐锚点，实际位置由 HAlign / VAlign 推导。
	 * 注意：这里会做一次文字测量；调用方若已缓存尺寸，应改用 DrawTextAtSize 避免重复测量。
	 */
	void DrawText(const FString& Text, const FVector2D& Position, const FSlateFontInfo& Font, const FLinearColor& Color,
	              EChartTextHAlign HAlign = EChartTextHAlign::Left, EChartTextVAlign VAlign = EChartTextVAlign::Top)
	{
		if (Text.IsEmpty()) return;
		DrawTextAtSize(Text, Position, MeasureText(Text, Font), Font, Color, HAlign, VAlign);
	}

	/** 已知文字尺寸的绘制版本，供布局阶段缓存过尺寸的场景使用 */
	void DrawTextAtSize(const FString& Text, const FVector2D& Position, const FVector2D& TextSize,
	                    const FSlateFontInfo& Font, const FLinearColor& Color,
	                    EChartTextHAlign HAlign = EChartTextHAlign::Left, EChartTextVAlign VAlign = EChartTextVAlign::Top)
	{
		if (Text.IsEmpty() || TextSize.IsNearlyZero()) return;

		FVector2D Offset = Position;
		if (HAlign == EChartTextHAlign::Center)      Offset.X -= TextSize.X * 0.5;
		else if (HAlign == EChartTextHAlign::Right)  Offset.X -= TextSize.X;
		if (VAlign == EChartTextVAlign::Middle)      Offset.Y -= TextSize.Y * 0.5;
		else if (VAlign == EChartTextVAlign::Bottom) Offset.Y -= TextSize.Y;

		// 多留 1 像素，避免测量误差导致末尾被裁
		const FVector2D PaintSize(TextSize.X + 1.0, TextSize.Y + 1.0);
		FSlateDrawElement::MakeText(
			OutDrawElements, static_cast<uint32>(ReserveLayer()),
			AllottedGeometry.ToPaintGeometry(PaintSize, FSlateLayoutTransform(Offset)),
			Text, Font, ESlateDrawEffect::None, Color);
	}

	/** 测量文字尺寸。Slate 未初始化（如专用服务器）时返回零 */
	static FVector2D MeasureText(const FString& Text, const FSlateFontInfo& Font);

	// ---------------------------------------------------------------- 渐变填充

	/**
	 * 渐变带：上边界是一条折线（局部坐标），下边界是固定的局部 Y。
	 * 颜色自上而下从 TopColor 插值到 BottomColor。
	 */
	void DrawGradientBand(const TArray<FVector2d>& TopPoints, double BottomY,
	                      const FLinearColor& TopColor, const FLinearColor& BottomColor);

	/** 垂直渐变矩形（背景用），基于 DrawGradientBand 实现 */
	void DrawVerticalGradient(const FBox2D& LocalRect, const FLinearColor& TopColor, const FLinearColor& BottomColor);

private:

	FSlateWindowElementList& OutDrawElements;
	const FGeometry& AllottedGeometry;
	int32 CurrentLayer;
	int32 ClipDepth = 0;
};