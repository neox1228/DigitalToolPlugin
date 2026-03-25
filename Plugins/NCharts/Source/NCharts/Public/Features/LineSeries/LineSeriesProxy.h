#pragma once

#include "CoreMinimal.h"
#include "Core/NChartTypes.h"
#include "Features/LineSeries/LineSeriesState.h"

class NCHARTS_API FLineSeriesProxy : public TSharedFromThis<FLineSeriesProxy>, public INChartProxy
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

private:
	void UpdateDerivedState();

	FLineSeriesState State;
	FOnStateChanged StateChanged;
};
