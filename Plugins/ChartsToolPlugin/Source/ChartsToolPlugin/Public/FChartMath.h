// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Layout/Geometry.h"
#include "FChartMath.generated.h"

UENUM(BlueprintType)
enum class EChartOrigin : uint8
{
	LeftBottom     UMETA(DisplayName="Left-Bottom"),
	CenterBottom   UMETA(DisplayName="Center-Bottom"),
	RightBottom    UMETA(DisplayName="Right-Bottom"),

	LeftCenter     UMETA(DisplayName="Left-Center"),
	Center         UMETA(DisplayName="Center"),
	RightCenter    UMETA(DisplayName="Right-Center"),

	LeftTop        UMETA(DisplayName="Left-Top"),
	CenterTop      UMETA(DisplayName="Center-Top"),
	RightTop       UMETA(DisplayName="Right-Top"),
};

USTRUCT(BlueprintType)
struct CHARTSTOOLPLUGIN_API FChartAxisSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	EChartOrigin Origin = EChartOrigin::LeftBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float XAxisMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float XAxisMax = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float YAxisMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float YAxisMax = 100.0f;
};

/**
 * 
 */

class CHARTSTOOLPLUGIN_API FChartMath
{
public:

	/** 禁止实例化（C++11 引入）：工具类不需要构造函数 */
	FChartMath() = delete;

	/**
	 * 通用最值查询函数
	 * @param DataArray 原始数组
	 * @param OutMin 输出最小值
	 * @param OutMax 输出最大值
	 */
	static void GetArrayRange(
		const TArray<FVector2D>& DataArray,
		FVector2D& OutMin,
		FVector2D& OutMax
		);

	/**
	 * 根据传入的坐标原点枚举计算当前原点的像素坐标
	 * @param InOrigin 原点位置枚举
	 * @param InGeometry 当前 Slate 控件的几何信息
	 * @return FVector2D 原点的像素坐标
	 */
	static FVector2D GetOriginLocation(
		const EChartOrigin& InOrigin,
		const FGeometry& InGeometry
		);

	/**
	 * 将传入的数据点位转换为本地 slate 像素坐标
	 * @param InData 原始数据（X, Y）
	 * @param InGeometry 当前 Slate 控件的几何信息
	 * @param InSettings 包含 Max/Min/Origin 等配置
	 * @return FVector2D 转换后的本地像素坐标
	 */
	static FVector2D DataToLocal(
		const FVector2D& InData,
		const FGeometry& InGeometry,
		const FChartAxisSettings& InSettings
		);

	/**
	 * 将输入的数据按照 X 值进行从小到大的排序
	 * @param InDataArray 原始数组
	 * @return TArray<FVector2D> 排序后的数据
	 */
	static void SortByValueX(
		TArray<FVector2D>& InDataArray
		);
	
	//~FChartMath() = delete;
};

