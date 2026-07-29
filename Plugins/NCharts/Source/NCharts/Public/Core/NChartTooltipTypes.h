// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"

/**
 * Tooltip 触发模式，对应 ECharts tooltip.trigger
 * - Item：命中最近图形元素（散点、折线点）
 * - Axis：按鼠标 X 轴位置聚合同图内全部数据序列
 */
UENUM(BlueprintType)
enum class EChartTooltipTrigger : uint8
{
	Item UMETA(DisplayName = "Item"),
	Axis UMETA(DisplayName = "Axis")
};

/** 单条 series 在 Tooltip 中展示的一项数据 */
struct NCHARTS_API FTooltipParam
{
	FName SeriesName = NAME_None;
	EChartFeatureType FeatureType = EChartFeatureType::LineSeries;
	int32 PointIndex = INDEX_NONE;
	FVector2D DataPoint = FVector2D::ZeroVector;
	FVector2D ScreenPoint = FVector2D::ZeroVector;
	FText DisplayText;
};

/**
 * INChartTooltipDataProvider —— 可向 Chart 级 Tooltip 提供数据的接口
 *
 * 实现此接口的 Proxy（如 LineSeries、PointBubbles）会在 PostBuildLink 阶段
 * 被 Tooltip 自动发现并参与命中检测与内容格式化，无需手动绑定。
 */
class NCHARTS_API INChartTooltipDataProvider
{
public:
	virtual ~INChartTooltipDataProvider() = default;

	virtual FName GetSeriesName() const = 0;
	virtual EChartFeatureType GetProviderFeatureType() const = 0;
	virtual FVector2D GetChartPadding() const = 0;
	virtual const TArray<FVector2D>& GetDataPoints() const = 0;

	/** 格式化该 series 在指定索引处的提示文本 */
	virtual FText FormatTooltipText(int32 PointIndex) const = 0;

	/** 至少需要多少数据点才能参与布局计算 */
	virtual int32 GetMinPointCountForLayout() const { return 1; }
};
