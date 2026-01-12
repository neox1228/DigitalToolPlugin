// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#include "ChartBase.h"
#include "FunnelChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

USTRUCT(BlueprintType)
struct FfunnelData
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float value = 0.0f;
public:
	FfunnelData()
	{}
	FfunnelData(FString value1, float value2)
		:name(value1)
		, value(value2)
	{

	}
};

USTRUCT(BlueprintType)
struct FfunnelSeries
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Chart|Series")
		float left = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Chart|Series")
		float width = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float min = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float max = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Chart|Series")
		float minSize = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1),Category = "Chart|Series")
		float maxSize = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		float gap = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series|label")
		bool label_show = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series|label")
		Position label_Position = Position::center;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		TArray<FfunnelData> data;
};



UCLASS()
class SIMPLECHARTS_API UFunnelChart : public UChartBase
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
		void SetSeries(FfunnelSeries SeriesData);

	//Series
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Series")
		FfunnelSeries Series;

private:
	//Chart Data
	TArray<TSharedPtr<FJsonValue>> series_json;
};
