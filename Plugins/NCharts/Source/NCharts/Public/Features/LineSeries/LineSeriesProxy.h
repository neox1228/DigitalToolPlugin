// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartCartesianScale.h"
#include "Core/NChartTooltipTypes.h"
#include "Core/NChartTypes.h"
#include "Features/LineSeries/LineSeriesState.h"

class NCHARTS_API FLineSeriesProxy : public TSharedFromThis<FLineSeriesProxy>, public INChartProxy, public INChartTooltipDataProvider, public INChartScaleConsumer
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	static TSharedRef<FLineSeriesProxy> CreateDemo();

	const FLineSeriesState& GetState() const;
	FOnStateChanged& OnStateChanged();

	void SetPoints(const TArray<FVector2D>& InPoints);
	void SetLineColor(const FLinearColor& InColor);
	void SetYLimit(float InLimit, bool bEnable);
	void SetPadding(const FVector2D& InPadding);

	virtual INChartTooltipDataProvider* GetTooltipDataProvider() override { return this; }
	virtual FName GetSeriesName() const override;
	virtual EChartFeatureType GetProviderFeatureType() const override;
	virtual FVector2D GetChartPadding() const override;
	virtual const TArray<FVector2D>& GetDataPoints() const override;
	virtual FText FormatTooltipText(int32 PointIndex) const override;
	virtual int32 GetMinPointCountForLayout() const override;

	virtual INChartScaleConsumer* GetScaleConsumer() override { return this; }
	virtual void SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale) override;
	virtual TSharedPtr<FNChartCartesianScale> GetCartesianScale() const override;

private:
	void UpdateDerivedState();

	FLineSeriesState State;
	TSharedPtr<FNChartCartesianScale> CartesianScale;
	FOnStateChanged StateChanged;
};
