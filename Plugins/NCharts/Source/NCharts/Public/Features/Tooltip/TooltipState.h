#pragma once

#include "CoreMinimal.h"

struct NCHARTS_API FTooltipState
{
	bool bEnableTooltip = true;
	bool bShowHoverLine = true;
	bool bShowHoverPoint = true;

	float ActivationDistance = 24.0f;
	float HoverPointRadius = 4.0f;

	FLinearColor HoverLineColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.35f);
	FLinearColor HoverPointColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	FLinearColor TooltipBackgroundColor = FLinearColor(0.08f, 0.08f, 0.08f, 0.90f);
	FLinearColor TooltipTextColor = FLinearColor::White;

	FVector2D TooltipOffset = FVector2D(12.0f, -12.0f);

	bool bHasHover = false;
	int32 HoveredIndex = INDEX_NONE;
	FVector2D HoveredDataPoint = FVector2D::ZeroVector;
	FVector2D HoveredScreenPoint = FVector2D::ZeroVector;
};
