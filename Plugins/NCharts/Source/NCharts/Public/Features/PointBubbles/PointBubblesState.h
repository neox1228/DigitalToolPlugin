#pragma once

#include "CoreMinimal.h"

struct NCHARTS_API FPointBubblesState
{
	TArray<FVector2D> Points;

	FVector2D Padding = FVector2D(16.0f, 12.0f);
	float PointRadius = 4.0f;
	float ActivationDistance = 20.0f;
	float BreathAmplitude = 2.5f;
	float BreathSpeed = 3.5f;

	FLinearColor PointColor = FLinearColor(0.95f, 0.95f, 0.98f, 0.95f);
	FLinearColor HoverPointColor = FLinearColor(1.0f, 0.45f, 0.2f, 1.0f);
	FLinearColor BubbleBackgroundColor = FLinearColor(0.08f, 0.08f, 0.08f, 0.92f);
	FLinearColor BubbleTextColor = FLinearColor::White;

	FVector2D BubbleOffset = FVector2D(0.0f, -12.0f);

	bool bHasHover = false;
	int32 HoveredIndex = INDEX_NONE;
	FVector2D HoveredDataPoint = FVector2D::ZeroVector;
	FVector2D HoveredScreenPoint = FVector2D::ZeroVector;
};
