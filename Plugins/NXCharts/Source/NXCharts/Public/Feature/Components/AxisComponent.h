// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChartComponent.h"
#include "AxisComponent.generated.h"

class UCartesianCoordModule;

UENUM(BlueprintType)
enum class EChartAxisType : uint8
{
	X, // X轴
	Y // Y轴
};

USTRUCT(BlueprintType)
struct FChartAxisRange
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AxisMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AxisMax;
	
	FChartAxisRange(){}
	
	FChartAxisRange(float InAxisMin, float InAxisMax) 
	: AxisMin(InAxisMin), AxisMax(InAxisMax){}
	
};

UCLASS()
class NXCHARTS_API UAxisComponent : public UChartComponent
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AxisComponent")
	EChartAxisType AxisType = EChartAxisType::X;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AxisComponent")
	FLinearColor AxisColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AxisComponent")
	float AxisThickness;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AxisComponent", meta = (InlineEditConditionToggle))
	bool bAutoAdjustRange = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AxisComponent", meta = (EditCondition = "bAutoAdjustRange"))
	FChartAxisRange AxisRange = FChartAxisRange(0, 100);
	
	virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry) override;
	virtual int32 HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const override;
	
protected:
	TArray<FVector2D> CalculateAxisPoints(const FGeometry& AllottedGeometry) const;
};
