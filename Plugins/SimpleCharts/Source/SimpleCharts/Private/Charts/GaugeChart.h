// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#include "ChartBase.h"
#include "GaugeChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

USTRUCT(BlueprintType)
struct FGaugeData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name="";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float value=0.0f;
public:
	FGaugeData()
	{}
	FGaugeData(FString value1, float value2)
		:name(value1)
		, value(value2)
	{

	}
};

USTRUCT(BlueprintType)
struct FGaugeSeries
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float min = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float max = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<FGaugeData> data;
};



UCLASS()
class SIMPLECHARTS_API UGaugeChart : public UChartBase
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
		void SetSeries(FGaugeSeries SeriesData);

	//Series
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FGaugeSeries Series;

private:
	//Chart Data
	TArray<TSharedPtr<FJsonValue>> series_json;

};
