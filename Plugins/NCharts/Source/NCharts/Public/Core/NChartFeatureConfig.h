// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NChartFeatureConfig.generated.h"

class INChartProxy;

/**
 * 坐标轴数据类型枚举
 * - Category：类目轴，使用离散字符串标签（如月份名称）
 * - Value：数值轴，使用连续数值范围自动缩放
 */
UENUM(BlueprintType)
enum class EAxisType : uint8
{
	Category UMETA(DisplayName = "Category"),
	Value UMETA(DisplayName = "Value")
};

/**
 * 图表功能模块类型枚举
 * 每个枚举值对应一个可独立注册、独立绘制的 Feature 单元。
 * 新增 Feature 时需在此扩展枚举，并在 FNChartRegistry 中完成注册。
 */
UENUM(BlueprintType)
enum class EChartFeatureType : uint8
{
	AxisX UMETA(DisplayName = "Axis X"),           // X 轴
	AxisY UMETA(DisplayName = "Axis Y"),           // Y 轴
	LineSeries UMETA(DisplayName = "Line Series"),   // 折线序列
	Tooltip UMETA(DisplayName = "Tooltip"),         // 悬浮提示
	PointBubbles UMETA(DisplayName = "Point Bubbles") // 气泡点标注
};

/**
 * UNChartFeatureConfigBase —— 所有 Feature 配置对象的抽象基类
 *
 * Config 层职责：
 * 1. 在编辑器 Details 面板中暴露可编辑属性（UPROPERTY）
 * 2. 支持蓝图读写（BlueprintReadWrite）
 * 3. 作为 ULineChartWidget::Features 数组的内嵌 Instanced 子对象持久化
 * 4. 通过 ApplyToProxy() 将配置同步到运行时 Proxy 状态
 *
 * 设计模式：Config-Proxy-View 三层分离中的「配置层」
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API UNChartFeatureConfigBase : public UObject
{
	GENERATED_BODY()

public:
	/** 返回当前 Config 对应的 Feature 类型，供 Registry 和 Widget 查找匹配 */
	virtual EChartFeatureType GetFeatureType() const PURE_VIRTUAL(UNChartFeatureConfigBase::GetFeatureType, return EChartFeatureType::AxisX;);

	/**
	 * 将当前 Config 中的属性值写入目标 Proxy
	 * @param InProxy 由 FNChartRegistry::CreateProxy 创建的运行时代理实例
	 */
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const {}
};
