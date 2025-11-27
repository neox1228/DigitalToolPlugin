// Fill out your copyright notice in the Description page of Project Settings.


#include "SChartAxes.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChartAxes::Construct(const FArguments& InArgs)
{
	Oringin = InArgs._Oringin;
	AxisColor = InArgs._AxisColor;
	AxisThickness = InArgs._AxisThickness;

	XAxisMin = InArgs._XAxisMin;
	XAxisMax = InArgs._XAxisMax;
	XAxisTicks = InArgs._XAxisTicks;
	YAxisMin = InArgs._YAxisMin;
	YAxisMax = InArgs._YAxisMax;
	YAxisTicks = InArgs._YAxisTicks;

	TickFontSize = InArgs._TickFontSize;
	ShowGrid = InArgs._ShowGrid;

	
}

FVector2D SChartAxes::GetOriginPosition(const FVector2D& Size) const
{
	switch (Oringin.Get())
	{
	case EChartOrigin::LeftBottom: return FVector2D(0, Size.Y);
	case EChartOrigin::LeftCenter: return FVector2D(0, Size.Y * 0.5f);
	case EChartOrigin::LeftTop: return FVector2D(0, 0);
		
	case EChartOrigin::CenterBottom: return FVector2D(Size.X * 0.5, Size.Y);
	case EChartOrigin::Center: return FVector2D(Size.X * 0.5, Size.Y * 0.5);
	case EChartOrigin::CenterTop: return FVector2D(Size.X * 0.5, 0);

	case EChartOrigin::RightBottom: return FVector2D(Size.X, Size.Y);
	case EChartOrigin::RightCenter: return FVector2D(Size.X, Size.Y * 0.5);
	case EChartOrigin::RightTop: return FVector2D(Size.X,0);

	default: return FVector2D(0, Size.Y);
	}
}

FVector2D SChartAxes::DataToLocal( const FVector2D& DataPoint) const
{

	FVector2D CurrentSize = GetOriginPosition(CacheGeometry.GetLocalSize());
	// 获取 widget大小
	FVector2D Size = CacheGeometry.GetLocalSize();
	float Width = Size.X;
	float Height = Size.Y;

	// 获取数据范围（放置除以 0 ）
	float minX = XAxisMin.Get();
	float maxX = XAxisMax.Get();
	float minY = YAxisMin.Get();
	float maxY = YAxisMax.Get();

	float rangeX = FMath::IsNearlyEqual(maxX, minX) ? 1.f : (maxX - minX);
	float rangeY = FMath::IsNearlyEqual(maxY, minY) ? 1.f : (maxY - minY);

	float nx = (DataPoint.X - minX) / rangeX;
	float ny = (DataPoint.Y - minY) / rangeY;
	
	float px = Width * nx;
	float py = Height * ny;

	FVector2D AxisX = (FVector2D(CacheGeometry.GetLocalSize().X, Size.Y) - Size).GetSafeNormal();// X轴正方向归一化向量
	FVector2D AxisY = (FVector2D(Size.X, 0) - Size).GetSafeNormal();// Y轴正方向归一化向量

	FVector2D OP = DataPoint - Size;

	float localX = FVector2D::DotProduct(OP, AxisX);
	float localY = FVector2D::DotProduct(OP, AxisY);

	if (localX > 0 && localY > 0)
	{
		px = Size.X + (Width - Size.X) * nx;
		py = Size.Y - (Size.Y - 0) * ny;
		
	}else if (localX < 0 && localY > 0)
	{
		px = Size.X - (Size.X - 0) * nx;
		py = Size.Y - (Size.Y - 0) * ny;
	}else if (localX < 0 && localY < 0)
	{
		px = Size.X - (Size.X - 0) * nx;
		py = Size.Y + (Height - Size.Y) * ny;
	}else if (localX > 0 && localY < 0)
	{
		px = Size.X + (Width - Size.X) * nx;
		py = Size.Y + (Height - Size.Y) * ny;
	}

	return FVector2D(px, py);

	
}

void SChartAxes::GetXRange(float& OutMin, float& OutMax) const
{
	OutMin = XAxisMin.Get();
	OutMax = XAxisMax.Get();
}

void SChartAxes::GetYRange(float& OutMin, float& OutMax) const
{
	OutMin = YAxisMin.Get();
	OutMax = YAxisMax.Get();
}

int32 SChartAxes::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                          FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                          bool bParentEnabled) const
{
	CacheGeometry = AllottedGeometry;
	FVector2D Size = GetOriginPosition(AllottedGeometry.GetLocalSize());
	//UE_LOG(LogTemp, Warning, TEXT("neo---Size_X:%d; Size_Y:%d"), Size.X, Size.Y);


	//X轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(0, Size.Y), Size, FVector2D(AllottedGeometry.GetLocalSize().X, Size.Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);

	//Y轴
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		{FVector2D(Size.X, 0), Size, FVector2D(Size.X, AllottedGeometry.GetLocalSize().Y)},
		ESlateDrawEffect::None,
		AxisColor.Get(),
		false,
		AxisThickness.Get()
	);

	
	
	return LayerId + 2;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
