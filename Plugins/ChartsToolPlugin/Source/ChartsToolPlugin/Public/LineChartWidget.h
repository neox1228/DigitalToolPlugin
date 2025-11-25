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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	FLinearColor AxisColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LineChart")
	float AxisThinkness;
	

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
		FVector2D TempChartPosion ;
		switch (ChartOrigin)
		{
			case EChartOrigin::LeftBottom:  TempChartPosion = FVector2D(0, 1);
			case EChartOrigin::LeftCenter: TempChartPosion = FVector2D(0, 0.5);
			case EChartOrigin::LeftTop: TempChartPosion = FVector2D(0, 0);
			
			case EChartOrigin::CenterBottom: TempChartPosion = FVector2D( 0.5, 1);
			case EChartOrigin::Center: TempChartPosion = FVector2D(0.5, 0.5);
			case EChartOrigin::CenterTop: TempChartPosion = FVector2D(0.5, 0);
	
			case EChartOrigin::RightBottom: TempChartPosion = FVector2D(1, 1);
			case EChartOrigin::RightCenter: TempChartPosion = FVector2D(1, 0.5);
			case EChartOrigin::RightTop: TempChartPosion = FVector2D(1,0);
	
			default: TempChartPosion = FVector2D(0, 1);
		}

		for (int i = 0; i < Data.Num(); i++)
		{
			
		}
		return Data;
	}

	
	UFUNCTION(BlueprintCallable, Category="LineChart")
	void SetData(TArray<FVector2D> InData)
	{
		Data = InData;
	}
};
