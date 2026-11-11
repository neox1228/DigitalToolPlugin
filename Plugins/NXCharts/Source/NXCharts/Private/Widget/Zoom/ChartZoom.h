#pragma once

#include "CoreMinimal.h"
#include "Layout/ChartSpace.h"

struct FChartFrame;

/** 一次滚轮缩放请求。实现方读 Frame、写 Override */
struct FChartZoomArgs
{
	FVector2D LocalPosition = FVector2D::ZeroVector;
	float WheelDelta = 0.0f;
	const FChartFrame* Frame = nullptr;
	FChartDomainOverride Override;
};

/**
 * 缩放策略接口。画布只持有一个实例并调用它，不认识任何图表类型。
 *
 * 加一种新的缩放（雷达径向、饼图缩放等）只需实现本接口并在画布注册时替换。
 */
class IChartZoom
{
public:
	virtual ~IChartZoom() = default;

	/** 返回 true 表示这次滚轮被消费，Args.Override 由调用方写回 */
	virtual bool HandleWheel(FChartZoomArgs& Args) = 0;
};