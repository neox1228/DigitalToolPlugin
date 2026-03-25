#pragma once

#include "CoreMinimal.h"
#include "Core/NChartTypes.h"
#include "Features/AxisX/AxisXState.h"

class NCHARTS_API FAxisXProxy : public TSharedFromThis<FAxisXProxy>, public INChartProxy
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	const FAxisXState& GetState() const;
	FOnStateChanged& OnStateChanged();

	void SetAxisColor(const FLinearColor& InColor);
	void SetAxisThickness(float InThickness);
	void SetPadding(const FVector2D& InPadding);

private:
	FAxisXState State;
	FOnStateChanged StateChanged;
};
