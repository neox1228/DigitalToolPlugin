// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartCartesianScale.h"
#include "Core/NChartTooltipTypes.h"
#include "Core/NChartTypes.h"
#include "Features/PointBubbles/PointBubblesState.h"

class NCHARTS_API FPointBubblesProxy : public TSharedFromThis<FPointBubblesProxy>, public INChartProxy, public INChartTooltipDataProvider, public INChartScaleConsumer
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	static TSharedRef<FPointBubblesProxy> CreateDemo();

	const FPointBubblesState& GetState() const;
	FOnStateChanged& OnStateChanged();

	void SetPoints(const TArray<FVector2D>& InPoints);
	void SetPadding(const FVector2D& InPadding);
	void SetPointRadius(float InRadius);
	void SetActivationDistance(float InDistance);
	void SetBreathAmplitude(float InAmplitude);
	void SetBreathSpeed(float InSpeed);
	void SetPointColor(const FLinearColor& InColor);
	void SetHoverPointColor(const FLinearColor& InColor);
	void SetBubbleBackgroundColor(const FLinearColor& InColor);
	void SetBubbleTextColor(const FLinearColor& InColor);
	void SetBubbleOffset(const FVector2D& InOffset);

	void SetHoverState(int32 InIndex, const FVector2D& InDataPoint, const FVector2D& InScreenPoint);
	void ClearHoverState();

	virtual INChartTooltipDataProvider* GetTooltipDataProvider() override { return this; }
	virtual FName GetSeriesName() const override;
	virtual EChartFeatureType GetProviderFeatureType() const override;
	virtual FVector2D GetChartPadding() const override;
	virtual const TArray<FVector2D>& GetDataPoints() const override;
	virtual FText FormatTooltipText(int32 PointIndex) const override;

	virtual INChartScaleConsumer* GetScaleConsumer() override { return this; }
	virtual void SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale) override;
	virtual TSharedPtr<FNChartCartesianScale> GetCartesianScale() const override;

private:
	FPointBubblesState State;
	TSharedPtr<FNChartCartesianScale> CartesianScale;
	FOnStateChanged StateChanged;
};
