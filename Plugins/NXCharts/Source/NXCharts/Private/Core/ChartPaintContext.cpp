// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ChartPaintContext.h"

#include "Application/SlateApplicationBase.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/DrawElementTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/SlateRenderer.h"

namespace
{
	/** 手写顶点：FSlateVertex 的默认构造函数不清零，这里逐字段写满 */
	FSlateVertex MakeChartVertex(const FVector2f& WindowPosition, const FVector2f& TexCoord, const FColor& Color)
	{
		FSlateVertex Vertex;
		Vertex.TexCoords[0] = TexCoord.X;
		Vertex.TexCoords[1] = TexCoord.Y;
		Vertex.TexCoords[2] = 1.0f;
		Vertex.TexCoords[3] = 1.0f;
		Vertex.MaterialTexCoords = FVector2f(0.0f, 0.0f);
		Vertex.Position = WindowPosition;
		Vertex.Color = Color;
		Vertex.SecondaryColor = FColor(0, 0, 0, 0);
		Vertex.PixelSize[0] = 0;
		Vertex.PixelSize[1] = 0;
		return Vertex;
	}
}

void FChartPaintContext::DrawGradientBand(const TArray<FVector2d>& TopPoints, double BottomY,
                                          const FLinearColor& TopColor, const FLinearColor& BottomColor)
{
	const int32 Count = TopPoints.Num();
	if (Count < 2)
	{
		return;
	}

	// 白笔刷只用来提供纹理与着色器资源，颜色全部来自顶点色
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetDefaultBrush();
	if (!WhiteBrush)
	{
		return;
	}

	const FSlateResourceHandle Handle = WhiteBrush->GetRenderingResource();
	const FSlateShaderResourceProxy* Proxy = Handle.GetResourceProxy();
	const FVector2f TexCoord = Proxy
		? (Proxy->StartUV + Proxy->SizeUV * 0.5f)
		: FVector2f(0.5f, 0.5f);

	const FColor TopColorLinear = TopColor.ToFColor(true);
	const FColor BottomColorLinear = BottomColor.ToFColor(true);

	TArray<FSlateVertex> Verts;
	TArray<SlateIndex> Indexes;
	Verts.Reserve(Count * 2);
	Indexes.Reserve((Count - 1) * 6);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const double X = TopPoints[Index].X;
		const double TopY = TopPoints[Index].Y;

		const FVector2D TopAbsolute = AllottedGeometry.LocalToAbsolute(FVector2D(X, TopY));
		const FVector2D BottomAbsolute = AllottedGeometry.LocalToAbsolute(FVector2D(X, BottomY));

		Verts.Add(MakeChartVertex(FVector2f(TopAbsolute.X, TopAbsolute.Y), TexCoord, TopColorLinear));
		Verts.Add(MakeChartVertex(FVector2f(BottomAbsolute.X, BottomAbsolute.Y), TexCoord, BottomColorLinear));
	}

	for (int32 Index = 0; Index < Count - 1; ++Index)
	{
		const SlateIndex V0 = static_cast<SlateIndex>(Index * 2);
		const SlateIndex V1 = static_cast<SlateIndex>(Index * 2 + 1);
		const SlateIndex V2 = static_cast<SlateIndex>(Index * 2 + 2);
		const SlateIndex V3 = static_cast<SlateIndex>(Index * 2 + 3);

		Indexes.Add(V0);
		Indexes.Add(V1);
		Indexes.Add(V2);

		Indexes.Add(V1);
		Indexes.Add(V3);
		Indexes.Add(V2);
	}

	FSlateDrawElement::MakeCustomVerts(OutDrawElements, static_cast<uint32>(ReserveLayer()),
	                                   Handle, Verts, Indexes, nullptr, 0, 0);
}

void FChartPaintContext::DrawVerticalGradient(const FBox2D& LocalRect, const FLinearColor& TopColor,
                                             const FLinearColor& BottomColor)
{
	if (LocalRect.Max.Y <= LocalRect.Min.Y || LocalRect.Max.X <= LocalRect.Min.X)
	{
		return;
	}

	TArray<FVector2d> TopPoints;
	TopPoints.Reserve(2);
	TopPoints.Add(LocalRect.Min);
	TopPoints.Add(FVector2d(LocalRect.Max.X, LocalRect.Min.Y));

	DrawGradientBand(TopPoints, LocalRect.Max.Y, TopColor, BottomColor);
}

FVector2D FChartPaintContext::MeasureText(const FString& Text, const FSlateFontInfo& Font)
{
	if (Text.IsEmpty())
	{
		return FVector2D::ZeroVector;
	}

	// 专用服务器或 Slate 尚未初始化时没有字体度量服务，退化为零尺寸
	if (!FSlateApplicationBase::IsInitialized())
	{
		return FVector2D::ZeroVector;
	}

	FSlateRenderer* Renderer = FSlateApplicationBase::Get().GetRenderer();
	if (!Renderer)
	{
		return FVector2D::ZeroVector;
	}

	const TSharedRef<FSlateFontMeasure> FontMeasure = Renderer->GetFontMeasureService();
	const FVector2f Measured = FontMeasure->Measure(Text, Font);
	return FVector2D(Measured.X, Measured.Y);
}
