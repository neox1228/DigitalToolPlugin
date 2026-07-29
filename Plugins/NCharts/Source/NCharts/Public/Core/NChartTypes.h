// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class INChartTooltipDataProvider;
class FNChartCartesianScale;

/**
 * INChartScaleConsumer —— 消费 Chart 级笛卡尔坐标系的 Feature
 */
class NCHARTS_API INChartScaleConsumer
{
public:
	virtual ~INChartScaleConsumer() = default;
	virtual void SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale) = 0;
	virtual TSharedPtr<FNChartCartesianScale> GetCartesianScale() const = 0;
};

/**
 * INChartProxy —— 所有图表 Feature 代理器的抽象基接口
 *
 * Proxy 层位于 Config（UObject 持久化配置）与 Widget（Slate 绘制）之间，
 * 负责持有运行时状态（State）、提供状态变更 API，并通过 OnStateChanged 委托
 * 通知绑定的 Slate Widget 触发重绘。
 *
 * 每个具体 Feature（如 AxisX、LineSeries）都会实现一个继承此接口的 Proxy 类。
 */
class NCHARTS_API INChartProxy
{
public:
	virtual ~INChartProxy() = default;

	/** 若该 Proxy 可向 Chart 级 Tooltip 提供数据，返回对应接口指针 */
	virtual INChartTooltipDataProvider* GetTooltipDataProvider() { return nullptr; }

	/** 若该 Proxy 使用 Chart 级坐标系，返回对应接口指针 */
	virtual INChartScaleConsumer* GetScaleConsumer() { return nullptr; }
};
