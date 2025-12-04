// Fill out your copyright notice in the Description page of Project Settings.


#include "SLineChartWidget.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLineChartWidget::Construct(const FArguments& InArgs)
{
	Data = InArgs._Data;
	LineColor = InArgs._LineColor;
	LineThinckness = InArgs._LineThinckness;
	CurrentSize = InArgs._CurrentSize;
}

FVector2D SLineChartWidget::DataToLocal(const FVector2D& DataPoint, const FGeometry& CacheGeometry) const
{
	//FVector2D CurrentSize = GetOriginPosition(CacheGeometry.GetLocalSize());
	// 获取 widget大小
	FVector2D Size = CacheGeometry.GetLocalSize();
	float Width = Size.X;
	float Height = Size.Y;
	//UE_LOG(LogTemp, Warning, TEXT("neo---坐标原点：X：%d，Y：%d"), Size.X, Size.Y);

	// 获取数据范围（放置除以 0 ）
	float minX = -1000;
	float maxX = 1000;
	float minY = -1000;
	float maxY = 1000;

	// 计算当前点位数据比例
	float nx = 0;
	float ny = 0;
	if (DataPoint.X > 0)
	{
		nx = DataPoint.X / maxX;
	}else if (DataPoint.X <= 0)
	{
		nx = DataPoint.X / minX;
	}
	if (DataPoint.Y > 0)
	{
		ny = DataPoint.Y / maxY;
	}else if (DataPoint.Y <= 0)
	{
		ny = DataPoint.Y / minY;
	}
	//UE_LOG(LogTemp, Warning, TEXT("neo---当前比例：X：%d，Y：%d"), nx, ny);

	// 根据当前画布大小映射当前数据到画布坐标系
	float px = Width * nx;
	float py = Height * ny;

	if (DataPoint.X > 0)
	{
		px = Size.X + (Width-Size.X) * nx;
	}else
	{
		px = Size.X + Size.X * nx;
	}

	if (DataPoint.Y > 0 )
	{
		py = Size.Y + Size.Y * ny;
	}
	else
	{
		py = Size.Y + (Height - Size.Y) * ny;
	}
	//UE_LOG(LogTemp, Warning, TEXT("neo---映射坐标：X：%d，Y：%d"), px, py);
	return FVector2D(px, py);
}

int32 SLineChartWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const TArray<FVector2D>& Points = Data.Get();
	UE_LOG(LogTemp, Warning, TEXT("neo---折线图大小：Size_X:%d; Size_Y:%d"), AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y);

	if (Points.Num() < 2)
	{
		return LayerId;
	}

	FVector2D Size = AllottedGeometry.GetLocalSize();

	for (int32 i = 0; i < Points.Num() - 1; i++)
	{
		FVector2D P1 = Points[i];
		FVector2D P2 = Points[i + 1];

		FSlateDrawElement::MakeLines(OutDrawElements,LayerId,AllottedGeometry.ToPaintGeometry(),{P1, P2},ESlateDrawEffect::None, LineColor.Get(),true
			,LineThinckness.Get());
	}
	return LayerId + 1;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
