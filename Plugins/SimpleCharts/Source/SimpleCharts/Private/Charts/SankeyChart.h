// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#include "ChartBase.h"
#include "SankeyChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

USTRUCT(BlueprintType)
struct Flinks
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString source = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString target = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float value = 0.0f;
public:
	Flinks() 
	{}

	Flinks(FString value1,FString value2,float value3)
		:source(value1)
		,target(value2)
		,value(value3)
	{}
};

USTRUCT(BlueprintType)
struct FSankeySeries
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<FString> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<Flinks> links;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Chart|Series")
		float left = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series|DataLabel")
		FSeriesLabel  DataLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		bool UseGradientColor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		FLinearGradient LinearGradient;
public:
	FSankeySeries()
	{}

	FSankeySeries(TArray<FString> value1, TArray<Flinks> value2,float value3)
		:data(value1)
		,links(value2)
		,left(value3)
	{}
};



UCLASS()
class SIMPLECHARTS_API USankeyChart : public UChartBase
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
		void SetSeries(FSankeySeries SeriesData);
	//Series
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FSankeySeries Series;

private:
	//Chart Data
	TSharedPtr<FJsonObject> series_json;
};
