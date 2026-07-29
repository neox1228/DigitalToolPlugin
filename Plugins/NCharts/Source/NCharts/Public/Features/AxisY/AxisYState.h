// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct NCHARTS_API FAxisYState
{
	bool bShowTicks = true;
	bool bShowLabels = true;
	FLinearColor AxisColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	float AxisThickness = 1.0f;
	FVector2D Padding = FVector2D(16.0f, 12.0f);
};
