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

UENUM(BlueprintType)
enum class EAxisType : uint8
{
	Numeric, // 数值模式
	Category // 分类模式（例如月份，名称）
};

enum EValue : uint8
{
	Max, // 最大值
	Min // 最小值
};

/**
 * 折线相关配置结构体
 * @param Data 源数据数组
 * @param SeriesColor 颜色
 * @param SeriesThinckness 线条粗细
 * @param Smooth 平滑开关
 */
USTRUCT(BlueprintType)
struct FSeriesSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SeriesColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SeriesThinckness = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Smooth;

	
	
};

/**
 * 折线相关配置结构体
 * @param OriginPosition 源数据数组
 * @param AxisX_Min 颜色
 * @param SeriesThinckness 线条粗细
 * @param Smooth 平滑开关
 */
USTRUCT(BlueprintType)
struct CHARTSTOOLPLUGIN_API FAxisLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FVector2D OriginPosition ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float AxisX_Min = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float AxisX_Max = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float AxisY_Min = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float AxisY_Max = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	int32 AxisX_OutPositiveTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	int32 AxisY_OutPositiveTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	int32 AxisX_OutNegativeTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	int32 AxisY_OutNegativeTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float AxisX_TickStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Chart")
	float AxisY_TickStep;
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
	 * 单折线最值查询函数
	 * @param DataArray 原始数组
	 * @param OutMaxX x轴最大值
	 * @param OutMaxY y轴最大值
	 */
	static void GetArrayRange(
		const TArray<FVector2D>& DataArray,
		float& OutMaxX,
		float& OutMaxY,
		EValue Option = EValue::Max
		);

	/**
	 * 多折线数据最值查询函数
	 * @param DataMap 折线数据Map
	 * @param OutMaxX x轴最大值
	 * @param OutMaxY y轴最大值
	 */
	static void GetArrayRange(
		const TMap<FString, FSeriesSetting>& DataMap,
		float& OutMaxX,
		float& OutMaxY,
		EValue Option = EValue::Max
		);

	/**
	 * 根据传入数据的最大最小值，计算当前坐标原点的像素坐标
	 * @param InMaxX
	 * @param InMaxY
	 * @param InMinX
	 * @param InMinY
	 * @return FVector2D 坐标原点像素坐标值
	 */
	static FVector2D GetOriginLocation(
		const float& InMaxX,
		const float& InMaxY,
		const float& InMinX,
		const float& InMinY,
		const FVector2D& InGeometrySize
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
		const FVector2D& InGeometrySize,
		const FAxisLayout& InSettings
		);

	/**
	 * 将输入的数据按照 X 值进行从小到大的排序
	 * @param InDataArray 原始数组
	 * @return TArray<FVector2D> 排序后的数据
	 */
	static void SortByValueX(
		TArray<FVector2D>& InDataArray
		);

	/**
	 * Nice Numbers 算法，用于将区间数的刻度步长始终落在1，2， 5， 10 及其倍数上
	 * @param MaxValue 当前半轴最大绝对值
	 * @param MaxValueByStep 计算出最优步长后的当前半轴最大绝对值
	 * @param MaxTicks 当前半轴刻度最大数量
	 * @return float 当前半轴最优刻度数量
	 */
	static float GetNiceStep(
		float MaxValue,
		int32 MaxTicks = 7
		);

	/**
	 * 根据传入数据轴区间，计算坐标系轴区间，正负轴刻度数量和步长
	 * @param AxisMaxValue 数据轴最大值
	 * @param AxisMinValue 数据轴最小值
	 * @param MaxTickNum 长半轴刻度数量
	 * @param MinTickNum 短半轴刻度数量
	 * @param Step 刻度步长
	 */
	static void CalculateAsymmetricAxisLayout(
		float& AxisMaxValue,
		float& AxisMinValue,
		int32& OutPositiveTicks,
		int32& OutNegativeTicks,
		float& OutStep,
		int32 MaxTicks = 7
		);

	/**
	 * 曲线计算公式
	 */
	static void CalculateCatmullRomPoints(const TArray<FVector2D>& RawPoints, TArray<FVector2D>& OutRenderPoints, int32 Segments = 30);
	//~FChartMath() = delete;
};

