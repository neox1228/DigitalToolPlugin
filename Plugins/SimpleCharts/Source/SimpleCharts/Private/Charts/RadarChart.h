// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#include "ChartBase.h"
#include "RadarChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

USTRUCT(BlueprintType)
struct FIndicator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|indicator")
		FString name = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|indicator")
		float max = 0.0f;
public:
	FIndicator()
	{}
	FIndicator(FString value1,float value2)
		:name(value1)
		,max(value2)
	{}
};

USTRUCT(BlueprintType)
struct FRadar
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|textStyle")
		bool circle = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Chart|textStyle")
		float radius = 0.6;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|textStyle")
		FColor color = FColor(255.0f, 255.0f, 255.0f, 255.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|textStyle")
		FColor backgroundColor = FColor(255.0f, 255.0f, 255.0f, 125.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|indicator")
		TArray<FIndicator> indicator;
};

USTRUCT(BlueprintType)
struct FData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<float> value;
public:
	FData()
	{}
	FData(FString value1, TArray<float> value2)
		:name(value1)
		, value(value2)
	{

	}
};

USTRUCT(BlueprintType)
struct FRadarSeries
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<FData> data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		bool UseGradientColor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		FLinearGradient LinearGradient;
};



UCLASS()
class SIMPLECHARTS_API URadarChart : public UChartBase
{
	GENERATED_UCLASS_BODY()

public:
	virtual void OnLoadCompleted() override;
	virtual void SynchronizeProperties() override;
#if WITH_EDITOR
	//~ Begin UWidget Interface
	virtual const FText GetPaletteCategory() override;
	//~ End UWidget Interface
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "Chart")
		void SetLegend(bool show, Position position, Orient orient, FLinearColor color, int32 fontSize);
	UFUNCTION(BlueprintCallable, Category = "Chart")
		void SetSeries(FRadarSeries SeriesData);
	UFUNCTION(BlueprintCallable, Category = "Chart")
		void SetRadar(FRadar RadarData);
	//Series
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FRadarSeries Series;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Radar")
		FRadar Radar;

private:
	//Chart Data
	TSharedPtr<FJsonObject> Radar_json;
	TArray<TSharedPtr<FJsonValue>> series_json;

};
