// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/BaseChartWidget.h"
#include "LineChartWidget.generated.h"

/**
 * ULineChartWidget —— 折线图预设模板（测试 / 示例用）
 *
 * 继承 UBaseChartWidget，在 InitializePresetFeatures 中注入
 * AxisX + AxisY + LineSeries + Tooltip 四个默认 Feature。
 *
 * SetPoints / SetYLimit / SetLineColor 为折线图场景的便捷 API，
 * 内部委托给基类的 Config + Proxy 双写逻辑。
 */
UCLASS()
class NCHARTS_API ULineChartWidget : public UBaseChartWidget
{
	GENERATED_BODY()

public:
	ULineChartWidget();

	UFUNCTION(BlueprintCallable, Category = "NCharts|LineChart")
	void SetPoints(const TArray<FVector2D>& InPoints);

	UFUNCTION(BlueprintCallable, Category = "NCharts|LineChart")
	void SetYLimit(float InLimit, bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "NCharts|LineChart")
	void SetLineColor(const FLinearColor& InColor);

protected:
	virtual void InitializePresetFeatures() override;
};
