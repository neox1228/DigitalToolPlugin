#pragma once

#include "CoreMinimal.h"
#include "Core/NChartTypes.h"
#include "Features/AxisY/AxisYState.h"

class NCHARTS_API FAxisYProxy : public TSharedFromThis<FAxisYProxy>, public INChartProxy
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnStateChanged);

	const FAxisYState& GetState() const;
	FOnStateChanged& OnStateChanged();

	void SetAxisColor(const FLinearColor& InColor);
	void SetAxisThickness(float InThickness);
	void SetPadding(const FVector2D& InPadding);

private:
	FAxisYState State;
	FOnStateChanged StateChanged;
};
