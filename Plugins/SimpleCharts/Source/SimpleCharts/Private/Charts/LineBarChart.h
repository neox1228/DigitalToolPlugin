// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#include "ChartBase.h"
#include "LineBarChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

UENUM(BlueprintType)
enum class ChartType :uint8
{
	bar,
	line
};

USTRUCT(BlueprintType)
struct FLineBarSeries
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<float> data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		ChartType type = ChartType::bar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series|DataLabel")
		FSeriesLabel DataLabel;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		bool UseGradientColor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		FLinearGradient LinearGradient;
};

UCLASS()
class SIMPLECHARTS_API ULineBarChart : public UChartBase
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
		void SetSeries(TArray<FLineBarSeries> SeriesData);

	//Series
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<FLineBarSeries> Series;

private:
	//Chart Data
	TArray<TSharedPtr<FJsonValue>> series_json;

};
