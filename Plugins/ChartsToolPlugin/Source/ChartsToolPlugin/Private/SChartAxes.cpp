// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"
#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChartAxes::Construct(const FArguments& InArgs)
{
	AxisLayout = InArgs._AxisLayout;
	
	AxisType = InArgs._AxisType;

	TickFont = InArgs._TickFont;
	
	AxisColor = InArgs._AxisColor;
	
	AxisThickness = InArgs._AxisThickness;

	GridDensity = InArgs._GridDensity;
	
	ShowGrid = InArgs._ShowGrid;
}

int32 SChartAxes::OnPaint(const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	if (ShowGrid.Get())
	{
		float MeridianNum = AllottedGeometry.GetLocalSize().X/GridDensity.Get();
		float ParallelNum = AllottedGeometry.GetLocalSize().Y/GridDensity.Get();
		for (int32 i = 0; i < MeridianNum; ++i)
		{
			//网格背景
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId ,
				AllottedGeometry.ToPaintGeometry(),
				{FVector2D{AllottedGeometry.GetLocalSize().X * (i / MeridianNum), 0}, FVector2D{AllottedGeometry.GetLocalSize().X * (i / MeridianNum), AllottedGeometry.GetLocalSize().Y}},
				ESlateDrawEffect::None,
				FLinearColor::Black,
				false,
				1.0f
				);
		}
		for (int32 i = 0; i < ParallelNum; ++i)
		{
			//网格背景
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				{FVector2D{0, AllottedGeometry.GetLocalSize().Y * (i / ParallelNum)}, FVector2D{ AllottedGeometry.GetLocalSize().X,AllottedGeometry.GetLocalSize().Y * (i / ParallelNum)}},
				ESlateDrawEffect::None,
				FLinearColor::Black,
				false,
				1.0f
				);
		}

	}
	
	// 获取坐标原点像素坐标值
	FVector2D OriginLocation = FChartMath::GetOriginLocation(AxisLayout.Get().AxisX_Max,AxisLayout.Get().AxisY_Max,
		AxisLayout.Get().AxisX_Min,AxisLayout.Get().AxisY_Min, AllottedGeometry.GetLocalSize());;
	FAxisLayout CurAxisLayout = AxisLayout.Get();

	float PixelStep = (AllottedGeometry.GetLocalSize().X - OriginLocation.X) / CurAxisLayout.AxisX_OutPositiveTicks;
	// X轴刻度和刻度值绘制
	for (int32 i = 1; i <= CurAxisLayout.AxisX_OutPositiveTicks; ++i)
	{
		TArray<FVector2D> TickPoints {FVector2D(OriginLocation.X + PixelStep * i,OriginLocation.Y), FVector2D(OriginLocation.X + PixelStep * i,OriginLocation.Y-8.0f)};
		FSlateDrawElement::MakeLines(
		OutDrawElements, LayerId, 
		AllottedGeometry.ToPaintGeometry(), 
		TickPoints, ESlateDrawEffect::None, 
		AxisColor.Get(), true, AxisThickness.Get()/2);

		// --- 绘制刻度数值 (Label) ---
		// 格式化文字：如果是整数则不显示小数点
		FString LabelText = (CurAxisLayout.AxisX_TickStep >= 1.0f) ? FString::FromInt(FMath::RoundToInt(i * CurAxisLayout.AxisX_TickStep)) : FString::Printf(TEXT("%.1f"), i * CurAxisLayout.AxisX_TickStep);
		FSlateFontInfo TestFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		TestFont.Size = 10;
		TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		FVector2D TextSize = FontMeasureService->Measure(LabelText, TestFont);
        
		// 计算右对齐位置
		FVector2D TextPos = FVector2D(OriginLocation.X + PixelStep * i ,OriginLocation.Y+8.0f) + FVector2D(-(TextSize.X/2), -TextSize.Y * 0.5f);
		FSlateDrawElement::MakeText(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
			LabelText, TestFont, ESlateDrawEffect::None, FLinearColor::White);
	}
	for (int32 i = 1; i <= CurAxisLayout.AxisX_OutNegativeTicks; ++i)
	{
		TArray<FVector2D> TickPoints {FVector2D(OriginLocation.X + (PixelStep * i * -1),OriginLocation.Y), FVector2D(OriginLocation.X + PixelStep * i * -1,OriginLocation.Y-8.0f)};
		FSlateDrawElement::MakeLines(
		OutDrawElements, LayerId, 
		AllottedGeometry.ToPaintGeometry(), 
		TickPoints, ESlateDrawEffect::None, 
		AxisColor.Get(), true, AxisThickness.Get()/2);

		// --- 绘制刻度数值 (Label) ---
		// 格式化文字：如果是整数则不显示小数点
		FString LabelText = (CurAxisLayout.AxisX_TickStep >= 1.0f) ? FString::FromInt(FMath::RoundToInt(-1 * i * CurAxisLayout.AxisX_TickStep)) : FString::Printf(TEXT("%.1f"), -1 * i * CurAxisLayout.AxisX_TickStep);
		FSlateFontInfo TestFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		TestFont.Size = 10;
		TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		FVector2D TextSize = FontMeasureService->Measure(LabelText, TestFont);
        
		// 计算右对齐位置
		FVector2D TextPos = FVector2D(OriginLocation.X + PixelStep * i * -1 ,OriginLocation.Y+8.0f) + FVector2D(-(TextSize.X/2), -TextSize.Y * 0.5f);
		FSlateDrawElement::MakeText(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
			LabelText, TestFont, ESlateDrawEffect::None, FLinearColor::White);
	}
	//UE_LOG(LogTemp, Warning, TEXT("neo---SlateSize(x:%f, y:%f ; )"), AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y);
	//int32 YTickCount = FChartMath::GetNiceStep(BoundY.Get());
	/*if (OriginLocation.X == 0 || OriginLocation.X == (AllottedGeometry.GetLocalSize().X / 2))
	{*/
		/*float PX = (AllottedGeometry.GetLocalSize().X - OriginLocation.X) / AxisLayout.Get().AxisX_TickStep;
		for (int32 i = 1; i <= PositiveTicks; ++i)
		{
			TArray<FVector2D> TickPoints {FVector2D(OriginLocation.X + Step * i,OriginLocation.Y), FVector2D(OriginLocation.X + Step * i,OriginLocation.Y-8.0f)};
			FSlateDrawElement::MakeLines(
			OutDrawElements, LayerId, 
			AllottedGeometry.ToPaintGeometry(), 
			TickPoints, ESlateDrawEffect::None, 
			AxisColor.Get(), true, AxisThickness.Get()/2);

			// --- 绘制刻度数值 (Label) ---
			// 格式化文字：如果是整数则不显示小数点
			FString LabelText = (Step >= 1.0f) ? FString::FromInt(FMath::RoundToInt(i * Step)) : FString::Printf(TEXT("%.1f"), i * Step);
			FSlateFontInfo TestFont = FCoreStyle::Get().GetFontStyle("NormalFont");
			TestFont.Size = 10;
			TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			FVector2D TextSize = FontMeasureService->Measure(LabelText, TestFont);
        
			// 计算右对齐位置
			FVector2D TextPos = FVector2D(OriginLocation.X + Step * i ,OriginLocation.Y+8.0f) + FVector2D(-(TextSize.X/2), -TextSize.Y * 0.5f);
			FSlateDrawElement::MakeText(
				OutDrawElements, LayerId,
				AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
				LabelText, TestFont, ESlateDrawEffect::None, FLinearColor::White);
		}*/
	/*}*/
	/*if (OriginLocation.X == AllottedGeometry.GetLocalSize().X || OriginLocation.X == (AllottedGeometry.GetLocalSize().X / 2))
	{
		float PX =  OriginLocation.X ;
		float Step = PX / XTickCount;
		float TextStep = BoundX.Get() / XTickCount;
		for (int32 i = 1; i <= XTickCount; ++i)
		{
			TArray<FVector2D> TickPoints {FVector2D(OriginLocation.X - Step * i,OriginLocation.Y), FVector2D(OriginLocation.X - Step * i,OriginLocation.Y-8.0f)};
			FSlateDrawElement::MakeLines(
			OutDrawElements, LayerId, 
			AllottedGeometry.ToPaintGeometry(), 
			TickPoints, ESlateDrawEffect::None, 
			AxisColor.Get(), true, AxisThickness.Get()/2);

			// --- 绘制刻度数值 (Label) ---
			// 格式化文字：如果是整数则不显示小数点
			FString LabelText = (TextStep >= 1.0f) ? FString::FromInt(FMath::RoundToInt(-i * TextStep)) : FString::Printf(TEXT("%.1f"), -i * TextStep);
			FSlateFontInfo TestFont = FCoreStyle::Get().GetFontStyle("NormalFont");
			TestFont.Size = 10;
			TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			FVector2D TextSize = FontMeasureService->Measure(LabelText, TestFont);
        
			// 计算右对齐位置
			FVector2D TextPos = FVector2D(OriginLocation.X - Step * i ,OriginLocation.Y+8.0f) + FVector2D(-(TextSize.X/2), -TextSize.Y * 0.5f);
			FSlateDrawElement::MakeText(
				OutDrawElements, LayerId,
				AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
				LabelText, TestFont, ESlateDrawEffect::None, FLinearColor::White);
		}
	}*/
	
	//X轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(0, OriginLocation.Y), OriginLocation, FVector2D(AllottedGeometry.GetLocalSize().X, OriginLocation.Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);

	//Y轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(OriginLocation.X, 0), OriginLocation, FVector2D(OriginLocation.X, AllottedGeometry.GetLocalSize().Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);
	

	
	
	return LayerId + 4;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
