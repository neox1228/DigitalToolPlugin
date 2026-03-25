#pragma once

#include "CoreMinimal.h"
#include "Core/NChartTypes.h"
#include "Features/Tooltip/TooltipState.h"

class FLineSeriesProxy;

class NCHARTS_API FTooltipProxy : public TSharedFromThis<FTooltipProxy>, public INChartProxy
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	const FTooltipState& GetState() const;
	FOnStateChanged& OnStateChanged();

	void SetTargetLineProxy(const TSharedPtr<FLineSeriesProxy>& InTargetLineProxy);
	TSharedPtr<FLineSeriesProxy> GetTargetLineProxy() const;

	void SetTooltipEnabled(bool bEnable);
	void SetShowHoverLine(bool bEnable);
	void SetShowHoverPoint(bool bEnable);
	void SetActivationDistance(float InDistance);
	void SetHoverLineColor(const FLinearColor& InColor);
	void SetHoverPointColor(const FLinearColor& InColor);
	void SetTooltipBackgroundColor(const FLinearColor& InColor);
	void SetTooltipTextColor(const FLinearColor& InColor);
	void SetHoverPointRadius(float InRadius);
	void SetTooltipOffset(const FVector2D& InOffset);

	void SetHoverState(int32 InIndex, const FVector2D& InDataPoint, const FVector2D& InScreenPoint);
	void ClearHoverState();

private:
	FTooltipState State;
	TWeakPtr<FLineSeriesProxy> TargetLineProxy;
	FOnStateChanged StateChanged;
};
