#include "Feature/Modules/CartesianCoordModule.h"
#include "Core/ChartDrawContext.h"
#include "Feature/Components/ChartComponent.h"
#include "Core/ChartUtils.h"

void UCartesianCoordModule::CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry)
{
	FBox2D Bounds = GetPaddedPlotBounds(AllottedGeometry);
	FLinearColor GridColor(0.3f, 0.3f, 0.3f, 0.5f);
    
	// 简单绘制一个白色背景的绘图区
	Ctx.DrawRect(Bounds.Min, Bounds.GetSize(), FLinearColor::White);
    
	// 可绘制网格线等，此处略
}

void UCartesianCoordModule::NotifyComponentDataChanged(UChartComponent* Component)
{
	// 如果开启自动调整，在这里可以检查所有绑定组件的数据范围并调整 CachedXRange/YRange
	// 然后广播模块更新，这里只简单调用基类广播
	BroadcastModuleUpdated();
}

FBox2D UCartesianCoordModule::GetPlotBounds(const FGeometry& AllottedGeometry) const
{
	return GetPaddedPlotBounds(AllottedGeometry);
}

FBox2D UCartesianCoordModule::GetPaddedPlotBounds(const FGeometry& AllottedGeometry) const
{
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	// 简单的内边距 50,30,30,50
	const float Left = 50, Top = 30, Right = Size.X - 30, Bottom = Size.Y - 50;
	return FBox2D(FVector2D(Left, Top), FVector2D(Right, Bottom));
}