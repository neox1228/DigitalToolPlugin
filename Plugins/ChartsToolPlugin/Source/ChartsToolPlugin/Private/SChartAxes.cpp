// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"
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
	
	// 1. 获取坐标原点像素坐标值
	FVector2D OriginLocation = FVector2D(0, AllottedGeometry.GetLocalSize().Y);
	
	OriginLocation = FChartMath::GetOriginLocation(AxisLayout.Get().AxisX_Max,AxisLayout.Get().AxisY_Max,
		AxisLayout.Get().AxisX_Min,AxisLayout.Get().AxisY_Min, AllottedGeometry.GetLocalSize());;
	FAxisLayout CurAxisLayout = AxisLayout.Get();

	// 2. 计算步长
	float PixelStepX = AllottedGeometry.GetLocalSize().X * (CurAxisLayout.AxisX_TickStep / (CurAxisLayout.AxisX_Max - CurAxisLayout.AxisX_Min));
	float PixelStepY = AllottedGeometry.GetLocalSize().Y * (CurAxisLayout.AxisY_TickStep / (CurAxisLayout.AxisY_Max - CurAxisLayout.AxisY_Min));
	UE_LOG(LogTemp, Warning, TEXT("neo---PixeStepX : %f, PixelStepY : %f"), PixelStepX, PixelStepY);


	// 3. 绘制四个方向的刻度
	// X 正轴
	FChartMath::DrawAxisTicks(OutDrawElements, LayerId, AllottedGeometry, OriginLocation, PixelStepX, CurAxisLayout.AxisX_TickStep, CurAxisLayout.AxisX_OutPositiveTicks, true, false, AxisThickness.Get(), TickFont.Get(), AxisColor.Get());
	// X 负轴
	FChartMath::DrawAxisTicks(OutDrawElements, LayerId, AllottedGeometry, OriginLocation, PixelStepX, CurAxisLayout.AxisX_TickStep, CurAxisLayout.AxisX_OutNegativeTicks, true, true, AxisThickness.Get(), TickFont.Get(), AxisColor.Get());
	// Y 正轴
	FChartMath::DrawAxisTicks(OutDrawElements, LayerId, AllottedGeometry, OriginLocation, PixelStepY, CurAxisLayout.AxisY_TickStep, CurAxisLayout.AxisY_OutPositiveTicks, false, false, AxisThickness.Get(), TickFont.Get(), AxisColor.Get());
	// Y 负轴
	FChartMath::DrawAxisTicks(OutDrawElements, LayerId, AllottedGeometry, OriginLocation, PixelStepY, CurAxisLayout.AxisY_TickStep, CurAxisLayout.AxisY_OutNegativeTicks, false, true, AxisThickness.Get(), TickFont.Get(), AxisColor.Get());

	
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
