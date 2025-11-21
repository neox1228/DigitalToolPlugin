// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SChartAxes.h"
#include "SLineChartWidget.h"
#include "Components/Widget.h"
#include "LineChartWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHARTSTOOLPLUGIN_API ULineChartWidget : public UWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	TArray<FVector2D> Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	FLinearColor LinearColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	float LineThickness = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	EChartOrigin ChartOrigin = EChartOrigin::LeftBottom;
	

protected:

	TSharedPtr<SLineChartWidget> SlateWidget;
	TSharedPtr<SChartAxes> ChartAxes;
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	void SyncProperties();
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION(BlueprintCallable, Category="LineChart")
	TArray<FVector2D> GetData()
	{
		return Data;
	}

	UFUNCTION(BlueprintCallable, Category="LineChart")
	void SetData(TArray<FVector2D> InData)
	{
		Data = InData;
	}
};
