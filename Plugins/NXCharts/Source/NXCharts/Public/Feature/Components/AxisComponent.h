// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChartComponent.h"
#include "AxisComponent.generated.h"

class UCartesianCoordModule;

UENUM(BlueprintType)
enum class EAxisType : uint8
{
	X, // X轴
	Y // Y轴
};

UCLASS()
class NXCHARTS_API UAxisComponent : public UChartComponent
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float AxisMin;
};
