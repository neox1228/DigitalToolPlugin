// Fill out your copyright notice in the Description page of Project Settings.


#include "FChartMath.h"


void FChartMath::GetArrayRange(const TArray<FVector2D>& DataArray, float& OutMaxX, float& OutMaxY, EValue Option)
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
		switch (Option)
		{
		case EValue::Max:
			OutMaxX = FMath::Max(OutMaxX, Temp.X);
			OutMaxY = FMath::Max(OutMaxY, Temp.Y);
			break;
		case EValue::Min:
			OutMaxX = FMath::Min(OutMaxX, Temp.X);
			OutMaxY = FMath::Min(OutMaxY, Temp.Y);
			break;
			
		}
		
	}
}

void FChartMath::GetArrayRange(const TMap<FString, FSeriesSetting>& DataMap, float& OutMaxX, float& OutMaxY, EValue Option)
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
		GetArrayRange(Elem.Value.Data,SingleMaxX, SingleMaxY, Option);
		switch (Option)
		{
		case EValue::Max:
			OutMaxX = FMath::Max(OutMaxX, SingleMaxX);
			OutMaxY = FMath::Max(OutMaxY, SingleMaxY);
			break;
		case EValue::Min:
			OutMaxX = FMath::Min(OutMaxX, SingleMaxX);
			OutMaxY = FMath::Min(OutMaxY, SingleMaxY);
			break;
			
		}
		
	}
}


FVector2D FChartMath::GetOriginLocation(const float& InMaxX, const float& InMaxY, const float& InMinX,
	const float& InMinY, const FVector2D& InGeometrySize)
{
	FVector2D GeometrySize = InGeometrySize;

	// 1. 计算数据 xy轴长度
	float LengthX = (InMaxX <= 0 ? 0 : InMaxX) - (InMinX >= 0 ? 0 : InMinX);
	float LengthY = (InMaxY <= 0 ? 0 : InMaxY) - (InMinY >= 0 ? 0 : InMinY);

	// 2. 计算坐标原点的像素坐标值的比例
	float ScaleX = (0 - (InMinX >= 0 ? 0 : InMinX)) / LengthX;
	float ScaleY = ((InMaxY <= 0 ? 0 : InMaxY) - 0) / LengthY;

	// 3. 计算当前坐标原点像素位置
	FVector2D Point;
	Point.X = GeometrySize.X * ScaleX;
	Point.Y = GeometrySize.Y * ScaleY;

	return Point;
	
}

FVector2D FChartMath::DataToLocal(const FVector2D& InData, const FVector2D& InGeometrySize,
                                  const FAxisLayout& InSettings)
{
	// 获取当前坐标原点的像素坐标值
	FVector2D CurrentOriginLocation = GetOriginLocation(InSettings.AxisX_Max, InSettings.AxisY_Max, InSettings.AxisX_Min, InSettings.AxisY_Min, InGeometrySize);

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
			RangeX = (InSettings.AxisX_Max - 0);
			RangeY = (InSettings.AxisY_Max - 0);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = (InGeometrySize.X - CurrentOriginLocation.X) / RangeX;
			PixelsPerUnitY = CurrentOriginLocation.Y / RangeY;
		}
		else
		{
			RangeX = (InSettings.AxisX_Max - 0);
			RangeY = (0 - InSettings.AxisY_Min);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = (InGeometrySize.X - CurrentOriginLocation.X) / RangeX;
			PixelsPerUnitY = (InGeometrySize.Y - CurrentOriginLocation.Y) / RangeY;
		}
	}
	else
	{
		if (InData.Y > 0)
		{
			RangeX = (0 - InSettings.AxisX_Min);
			RangeY = (InSettings.AxisY_Max - 0);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = CurrentOriginLocation.X / RangeX;
			PixelsPerUnitY = CurrentOriginLocation.Y / RangeY;
		}
		else
		{
			RangeX = (0 - InSettings.AxisX_Min);
			RangeY = (0 - InSettings.AxisY_Min);
			RangeX = FMath::IsNearlyZero(RangeX) ? 1.0f : RangeX;
			RangeY = FMath::IsNearlyZero(RangeY) ? 1.0f : RangeY;
			PixelsPerUnitX = CurrentOriginLocation.X / RangeX;
			PixelsPerUnitY = (InGeometrySize.Y - CurrentOriginLocation.Y) / RangeY;
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

float FChartMath::GetNiceStep(float MaxValue, int32 MaxTicks)
{
	
	// 1. 获取数量级（Magnitude）
	float Exponent = FMath::FloorToFloat(FMath::LogX(10, MaxValue));
	float Magnitude = FMath::Pow(10.f, Exponent);

	// 2. 提取有效数字（即 将数字映射到 1~10 之间）
	float Fraction = MaxValue / Magnitude;

	// 3. 步长种子库
	const TArray<float> NiceSteps = { 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
    
	float SelectedStepSeed = 1.0f;
	int32 BestTickCount = 0;

	// 4. 寻找最优步长种子
	for (float Candidate : NiceSteps)
	{
		// 计算当前步长种子下的刻度数 (向上取整)
		int32 CurrentTickCount = FMath::CeilToInt(Fraction / Candidate);

		// 如果刻度数在允许范围内
		if (CurrentTickCount <= MaxTicks)
		{
			// 因为 NiceSteps 是从小到大排列的，
			// 第一次符合条件的 Candidate 产生的刻度数必然是“最多且最密”的
			SelectedStepSeed = Candidate;
			BestTickCount = CurrentTickCount;
			break; 
		}
	}
	
	return SelectedStepSeed * Magnitude;
}

void FChartMath::CalculateAsymmetricAxisLayout(float& AxisMaxValue, float& AxisMinValue, int32& OutPositiveTicks,
	int32& OutNegativeTicks, float& OutStep, int32 MaxTicks)
{
	// 1. 以绝对值最大的一侧作为步长基准
	float MajorValue = FMath::Max(FMath::Abs(AxisMaxValue), FMath::Abs(AxisMinValue));
	if(MajorValue == 0) return;

	// 2. 调用 NiceStep 算法计算全局唯一的 OutStep
	OutStep = GetNiceStep(MajorValue, MaxTicks);

	// 3. 根据全局步长，分别计算正负轴需要的刻度数量
	OutPositiveTicks = (AxisMaxValue > 0) ? FMath::CeilToInt(AxisMaxValue / OutStep) : 0;
	OutNegativeTicks = (AxisMinValue < 0) ? FMath::CeilToInt(FMath::Abs(AxisMinValue) / OutStep) : 0;
	//UE_LOG(LogTemp, Warning, TEXT("neo---正半轴刻度数：%f; 负半轴刻度数： %f"), OutPositiveTicks, OutNegativeTicks);


	// 4. 特殊处理：如果某个半轴过小保留一个刻度范围
	if (AxisMaxValue * AxisMinValue < 0)
	{
		if(OutNegativeTicks == 0 && AxisMinValue < 0)
		{
			OutNegativeTicks = 1;
		}
		if (OutPositiveTicks == 0 && AxisMaxValue > 0)
		{
			OutPositiveTicks = 1;
		}
	}

	// 5. 根据计算出的步长和刻度数，计算轴最大最小值
	/*if (AxisMaxValue <= 0)
	{
		AxisMinValue = -1 * OutNegativeTicks * OutStep;
	}
	else
	{
		AxisMaxValue = OutPositiveTicks * OutStep;
	}*/
	AxisMinValue = -1 * OutNegativeTicks * OutStep;
	AxisMaxValue = OutPositiveTicks * OutStep;
}

