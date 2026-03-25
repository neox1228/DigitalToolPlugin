#pragma once

#include "CoreMinimal.h"

struct NCHARTS_API FLineSeriesState
{
	TArray<FVector2D> Points;

	FLinearColor BaseLineColor = FLinearColor(0.15f, 0.65f, 1.0f, 1.0f);
	FLinearColor OverLimitColor = FLinearColor(1.0f, 0.35f, 0.2f, 1.0f);
	FLinearColor ActiveLineColor = BaseLineColor;

	float LineThickness = 2.0f;
	FVector2D Padding = FVector2D(16.0f, 12.0f);

	bool bOverLimit = false;
	bool bUseLimit = false;
	float YLimit = 0.0f;
};
