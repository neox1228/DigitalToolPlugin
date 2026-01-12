// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#include "ChartBase.h"
#include "PieChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

USTRUCT(BlueprintType)
struct FPieSeries
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Chart|Series")
		float radius = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		bool label_show = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		bool labelLine_show = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		bool RoseType = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TMap<FString,float> data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		bool UseGradientColor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|LinearGradient")
		FLinearGradient LinearGradient;
};



UCLASS()
class SIMPLECHARTS_API UPieChart : public UChartBase
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
		void SetLegend(bool show, Position position, Orient orient, FLinearColor  color, int32 fontSize);
	UFUNCTION(BlueprintCallable, Category = "Chart")
		void SetSeries(FPieSeries SeriesData);

	//Series
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FPieSeries Series;

private:
	//Chart Data
	TArray<TSharedPtr<FJsonValue>> series_json;

};
