// Fill out your copyright notice in the Description page of Project Settings.


#include "FChartMath.h"


void FChartMath::GetArrayRange(const TArray<FVector2D>& DataArray, float& OutMaxX, float& OutMaxY)
{
	// 1. 防御性检查：如果数组为空，直接返回。防止DataArray[0] 崩溃
	if (DataArray.Num() == 0)
	{
		OutMaxX = OutMaxY = 0;
		return;
	}

	// 2. 初始化
	OutMaxX = OutMaxY = 0;

	// 3. 使用 for 循环，遍历查询范围
	for (int i = 0; i < DataArray.Num(); i++)
	{
		const FVector2D& Temp = DataArray[i];

		// 4. 使用 FMath::Min/Max,
		OutMaxX = FMath::Max(OutMaxX, FMath::Abs(Temp.X));
		OutMaxY = FMath::Max(OutMaxY, FMath::Abs(Temp.Y));
	}
}

void FChartMath::GetArrayRange(const TMap<FString, FSeriesSetting>& DataMap, float& OutMaxX, float& OutMaxY)
{
	// 1. 防御性检查：如果数组为空，直接返回。防止DataArray[0] 崩溃
	if (DataMap.Num() == 0)
	{
		OutMaxX = OutMaxY = 0;
		return;
	}

	// 2. 初始化
	OutMaxX = OutMaxY = 0;

	// 3. 遍历查找 xy轴最大值
	for (auto& Elem : DataMap)
	{
		float SingleMaxX;
		float SingleMaxY;
		GetArrayRange(Elem.Value.Data,SingleMaxX, SingleMaxY);
		OutMaxX = FMath::Max(OutMaxX, SingleMaxX);
		OutMaxY = FMath::Max(OutMaxY, SingleMaxY);
	}
}

FVector2D FChartMath::GetOriginLocation(const EChartOrigin& InOrigin, const FGeometry& InGeometry)
{
	switch (InOrigin)
	{
		case EChartOrigin::LeftBottom:
			return FVector2D(0, InGeometry.GetLocalSize().Y);

		case EChartOrigin::LeftCenter:
			return FVector2D(0, InGeometry.GetLocalSize().Y/2);

		case EChartOrigin::LeftTop:
			return FVector2D(0, 0);

		case EChartOrigin::CenterBottom:
			return FVector2D(InGeometry.GetLocalSize().X/2, InGeometry.GetLocalSize().Y);

		case EChartOrigin::Center:
			return FVector2D(InGeometry.GetLocalSize().X/2, InGeometry.GetLocalSize().Y/2);

		case EChartOrigin::CenterTop:
			return FVector2D(InGeometry.GetLocalSize().X/2, 0);

		case EChartOrigin::RightBottom:
			return FVector2D(InGeometry.GetLocalSize().X, InGeometry.GetLocalSize().Y);

		case EChartOrigin::RightCenter:
			return FVector2D(InGeometry.GetLocalSize().X, InGeometry.GetLocalSize().Y/2);

		case EChartOrigin::RightTop:
			return FVector2D(InGeometry.GetLocalSize().X, 0);
	}
	return FVector2D::ZeroVector;
}

FVector2D FChartMath::DataToLocal(const FVector2D& InData, const FGeometry& InGeometry,
                                  const FChartAxisSettings& InSettings)
{
	// 获取当前坐标原点的像素坐标值
	FVector2D CurrentOriginLocation = GetOriginLocation(InSettings.Origin, InGeometry);

	// 计算数据跨度（防止除以 0）
	float RangeX = 0;
	float RangeY = 0;

	// 计算每每单位数据对应的像素比例
	/*float PixelsPerUnitX = InGeometry.GetLocalSize().X / RangeX;
	float PixelsPerUnitY = InGeometry.GetLocalSize().Y / RangeY;*/

	float PixelsPerUnitX = 0;
	float PixelsPerUnitY = 0;

	if (InData.X > 0)
	{
		if (InData.Y > 0)
		{
			RangeX = (InSettings.XAxisMax - 0);
			RangeY = (InSettings.YAxisMax - 0);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = (InGeometry.GetLocalSize().X - CurrentOriginLocation.X) / RangeX;
			PixelsPerUnitY = CurrentOriginLocation.Y / RangeY;
		}
		else
		{
			RangeX = (InSettings.XAxisMax - 0);
			RangeY = (0 - InSettings.YAxisMin);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = (InGeometry.GetLocalSize().X - CurrentOriginLocation.X) / RangeX;
			PixelsPerUnitY = (InGeometry.GetLocalSize().Y - CurrentOriginLocation.Y) / RangeY;
		}
	}
	else
	{
		if (InData.Y > 0)
		{
			RangeX = (0 - InSettings.XAxisMin);
			RangeY = (InSettings.YAxisMax - 0);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = CurrentOriginLocation.X / RangeX;
			PixelsPerUnitY = CurrentOriginLocation.Y / RangeY;
		}
		else
		{
			RangeX = (0 - InSettings.XAxisMin);
			RangeY = (0 - InSettings.YAxisMin);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = CurrentOriginLocation.X / RangeX;
			PixelsPerUnitY = (InGeometry.GetLocalSize().Y - CurrentOriginLocation.Y) / RangeY;
		}
	}

	// 根据比例计算数据在控件空间中的投影坐标（以控件左上角为原点）
	float MappingX = InData.X * PixelsPerUnitX;
	float MappingY = InData.Y * PixelsPerUnitY;

	// 应用 Slate 坐标系转换
	float FinalX = CurrentOriginLocation.X + MappingX ;
	float FinalY = CurrentOriginLocation.Y - MappingY ;

	return FVector2D(FinalX, FinalY);
	
}

void FChartMath::SortByValueX(TArray<FVector2D>& InDataArray)
{
	// 1. 防御性检查：如果数组为空，直接返回。防止InDataArray[0] 崩溃
	if (InDataArray.Num() == 0)
	{
		return ;
	}

	// 2. 使用插入排序对输入数组进行排序
	for(int32 i = 1; i < InDataArray.Num(); ++i)
	{
		FVector2D Key = InDataArray[i];
		int32 j = i - 1;
		while (j >= 0 && InDataArray[j].X > Key.X)
		{
			InDataArray[j + 1] = InDataArray[j];
			j--;
		}
		InDataArray[j + 1] = Key;
	}
}

float FChartMath::GetNiceStep(float MaxValue, int32 DesiredTicks)
{
	if (MaxValue <= 0)
	{
		return 1.f;
	}

	// 1. 计算原始步长
	float RawStep = MaxValue / DesiredTicks;

	// 2. 获取数量级（对数）
	float Exponent = FMath::FloorToFloat(FMath::Loge(RawStep));
	float Fraction = RawStep / FMath::Pow(10.f, Exponent);

	// 3. 规范化到 1, 2, 5阶梯
	float NiceFraction;
	if (Fraction < 1.5f) NiceFraction = 1.f;
	else if (Fraction < 3.f) NiceFraction = 2.f;
	else if (Fraction < 7.f) NiceFraction = 5.f;
	else NiceFraction = 10.f;

	return NiceFraction * FMath::Pow(10.f, Exponent);
}

