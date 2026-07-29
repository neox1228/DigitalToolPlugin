// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NChartFeatureConfig.h"
#include "PointBubblesFeatureConfig.generated.h"

/**
 * UPointBubblesFeatureConfig —— 气泡点标注编辑器/蓝图配置
 *
 * 已注册到 Registry，但默认 ULineChartWidget 未包含此 Feature，
 * 需手动向 Features 数组添加 EChartFeatureType::PointBubbles 条目。
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class NCHARTS_API UPointBubblesFeatureConfig : public UNChartFeatureConfigBase
{
	GENERATED_BODY()

public:
	UPointBubblesFeatureConfig();

	virtual EChartFeatureType GetFeatureType() const override;
	virtual void ApplyToProxy(const TSharedRef<INChartProxy>& InProxy) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	TArray<FVector2D> Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	FVector2D Padding = FVector2D(16.0f, 12.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	float PointRadius = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	float ActivationDistance = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	float BreathAmplitude = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	float BreathSpeed = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	FLinearColor PointColor = FLinearColor(0.95f, 0.95f, 0.98f, 0.95f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	FLinearColor HoverPointColor = FLinearColor(1.0f, 0.45f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	FLinearColor BubbleBackgroundColor = FLinearColor(0.08f, 0.08f, 0.08f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	FLinearColor BubbleTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NCharts|PointBubbles")
	FVector2D BubbleOffset = FVector2D(0.0f, -12.0f);
};
