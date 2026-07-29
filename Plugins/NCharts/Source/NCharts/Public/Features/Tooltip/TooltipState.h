// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartTooltipTypes.h"

struct NCHARTS_API FTooltipState
{
	bool bEnableTooltip = true;
	bool bShowHoverLine = true;
	bool bShowHoverPoint = true;

	EChartTooltipTrigger Trigger = EChartTooltipTrigger::Item;

	float ActivationDistance = 24.0f;
	float HoverPointRadius = 4.0f;

	FLinearColor HoverLineColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.35f);
	FLinearColor HoverPointColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	FLinearColor TooltipBackgroundColor = FLinearColor(0.08f, 0.08f, 0.08f, 0.90f);
	FLinearColor TooltipTextColor = FLinearColor::White;

	FVector2D TooltipOffset = FVector2D(12.0f, -12.0f);

	bool bHasHover = false;

	/** Axis 模式下垂直参考线的屏幕 X 坐标 */
	float AxisScreenX = 0.0f;

	FVector2D DrawMin = FVector2D::ZeroVector;
	FVector2D DrawMax = FVector2D::ZeroVector;

	/** Item 模式通常 1 条，Axis 模式可包含多条 series */
	TArray<FTooltipParam> ActiveParams;
};
