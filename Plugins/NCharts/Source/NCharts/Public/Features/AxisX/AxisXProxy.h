// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartCartesianScale.h"
#include "Core/NChartTypes.h"
#include "Features/AxisX/AxisXState.h"

class NCHARTS_API FAxisXProxy : public TSharedFromThis<FAxisXProxy>, public INChartProxy, public INChartScaleConsumer
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	const FAxisXState& GetState() const;
	FOnStateChanged& OnStateChanged();

	void SetAxisColor(const FLinearColor& InColor);
	void SetAxisThickness(float InThickness);
	void SetPadding(const FVector2D& InPadding);
	void SetShowTicks(bool bEnable);
	void SetShowLabels(bool bEnable);

	virtual INChartScaleConsumer* GetScaleConsumer() override { return this; }
	virtual void SetCartesianScale(const TSharedPtr<FNChartCartesianScale>& InScale) override;
	virtual TSharedPtr<FNChartCartesianScale> GetCartesianScale() const override;

private:
	FAxisXState State;
	TSharedPtr<FNChartCartesianScale> CartesianScale;
	FOnStateChanged StateChanged;
};
