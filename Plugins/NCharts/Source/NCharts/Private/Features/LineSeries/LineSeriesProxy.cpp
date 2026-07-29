// Copyright NCharts Plugin. All Rights Reserved.
// FLineSeriesProxy 实现：折线数据管理与派生状态计算

#include "Features/LineSeries/LineSeriesProxy.h"

#include "Core/NChartCartesianScale.h"
#include "Core/NChartMathLib.h"

TSharedRef<FLineSeriesProxy> FLineSeriesProxy::CreateDemo()
{
	TSharedRef<FLineSeriesProxy> Proxy = MakeShared<FLineSeriesProxy>();

	// 演示数据：7 个点的折线，其中部分超过 YLimit=25
	TArray<FVector2D> DemoPoints;
	DemoPoints.Add(FVector2D(0.0, 10.0));
	DemoPoints.Add(FVector2D(1.0, 18.0));
	DemoPoints.Add(FVector2D(2.0, 12.0));
	DemoPoints.Add(FVector2D(3.0, 26.0));
	DemoPoints.Add(FVector2D(4.0, 20.0));
	DemoPoints.Add(FVector2D(5.0, 32.0));
	DemoPoints.Add(FVector2D(6.0, 28.0));

	Proxy->SetPoints(DemoPoints);
	Proxy->SetYLimit(25.0f, true);
	return Proxy;
}

const FLineSeriesState& FLineSeriesProxy::GetState() const
{
	return State;
}

FLineSeriesProxy::FOnStateChanged& FLineSeriesProxy::OnStateChanged()
{
	return StateChanged;
}

void FLineSeriesProxy::SetPoints(const TArray<FVector2D>& InPoints)
{
	State.Points = InPoints;
	NChartMathLib::SortPointsByX(State.Points);
	UpdateDerivedState();
	StateChanged.Broadcast();
}

void FLineSeriesProxy::SetLineColor(const FLinearColor& InColor)
{
	State.BaseLineColor = InColor;
	UpdateDerivedState();
	StateChanged.Broadcast();
}

void FLineSeriesProxy::SetYLimit(float InLimit, bool bEnable)
{
	State.YLimit = InLimit;
	State.bUseLimit = bEnable;
	UpdateDerivedState();
	StateChanged.Broadcast();
}

void FLineSeriesProxy::SetPadding(const FVector2D& InPadding)
{
	State.Padding = InPadding;
	StateChanged.Broadcast();
}

void FLineSeriesProxy::UpdateDerivedState()
{
	// 检测是否存在超限数据点
	State.bOverLimit = false;
	if (State.bUseLimit)
	{
		for (const FVector2D& Point : State.Points)
		{
			if (Point.Y > State.YLimit)
			{
				State.bOverLimit = true;
				break;
			}
		}
	}

	// 根据超限状态选择实际绘制颜色
	State.ActiveLineColor = State.bOverLimit ? State.OverLimitColor : State.BaseLineColor;
}

FName FLineSeriesProxy::GetSeriesName() const
{
	return TEXT("Line Series");
}

EChartFeatureType FLineSeriesProxy::GetProviderFeatureType() const
{
	return EChartFeatureType::LineSeries;
}

FVector2D FLineSeriesProxy::GetChartPadding() const
{
	return State.Padding;
}

const TArray<FVector2D>& FLineSeriesProxy::GetDataPoints() const
{
	return State.Points;
}

FText FLineSeriesProxy::FormatTooltipText(int32 PointIndex) const
{
	if (!State.Points.IsValidIndex(PointIndex))
	{
		return FText::GetEmpty();
	}

	const FVector2D& Point = State.Points[PointIndex];
	return FText::Format(
		FText::FromString(TEXT("{0}\nX: {1}\nY: {2}")),
		FText::FromName(GetSeriesName()),
		FText::AsNumber(Point.X),
		FText::AsNumber(Point.Y));
}

int32 FLineSeriesProxy::GetMinPointCountForLayout() const
{
	return 1;
}

void FLineSeriesProxy::SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale)
{
	CartesianScale = InScale;
	StateChanged.Broadcast();
}

TSharedPtr<FNChartCartesianScale> FLineSeriesProxy::GetCartesianScale() const
{
	return CartesianScale;
}
