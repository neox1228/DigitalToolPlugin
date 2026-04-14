#pragma once

#include "CoreMinimal.h"
#include "Core/NChartTypes.h"
#include "Features/PointBubbles/PointBubblesState.h"

class NCHARTS_API FPointBubblesProxy : public TSharedFromThis<FPointBubblesProxy>, public INChartProxy
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

private:
	FPointBubblesState State;
	FOnStateChanged StateChanged;
};
