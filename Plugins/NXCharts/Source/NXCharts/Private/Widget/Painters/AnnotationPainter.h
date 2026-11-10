#pragma once

#include "CoreMinimal.h"
#include "Widget/ChartPainter.h"

/**
 * 注解组件：阈值线 / 垂直线 / 色带。
 *
 * 这是一次扩展性验收：新增它只动了三处
 *   1. 本文件（新组件）
 *   2. Spec 里的 Annotations 数组 + FChartAnnotationSpec
 *   3. 画布里一行注册
 * OnPaint、Frame、布局、其它组件都没有改。
 *
 * 层序：Annotation —— 画在网格之上、曲线之下，不遮挡数据。
 */
class FAnnotationPainter : public FChartPainter
{
public:
	virtual EChartLayer GetLayer() const override;
	virtual bool IsEnabled(const FChartPaintArgs& Args) const override;
	virtual void Paint(const FChartPaintArgs& Args) const override;
};