// Copyright 2020 LHM. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SlateBrushAsset.h"
#if WITH_EDITOR
#include "SChartWebBrowser.h"
#else
#include "Runtime/WebBrowser/Public/SWebBrowser.h"
#endif
#include "CustomChart.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadEnd);
/**
 *
 */

UCLASS()
class SIMPLECHARTS_API UCustomChart : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	FString ThemeToString();
	void OnLoadCompleted();
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	//~ Begin UWidget Interface
	virtual const FText GetPaletteCategory() override;
	//~ End UWidget Interface
#endif
	UFUNCTION(BlueprintCallable, Category = "Chart")
		void SetTheme(ChartTheme ChartTheme);
	//OtherCode is code that can be executed otherwise
	//InOption can be a JSON or a object 
	UFUNCTION(BlueprintCallable, Category = "Chart")
		void SetOption(FString InOption,FString OtherCode = "");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
		bool useDirtyRect = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
		ChartTheme Theme;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
		FString Option = "{\"xAxis\": {\"data\" : [\"Mon\", \"Tue\", \"Wed\", \"Thu\", \"Fri\",\"Sat\", \"Sun\"]},\"yAxis\" : {\"type\": \"value\"},\"series\" : [{\"data\": [820, 932, 901, 934, 1290, 1330, 1320],\"type\" : \"line\"}]}";

	void SetOptionByJson(FString json);
	void SetOptionByObject(FString Object, FString OtherCode);
protected:
	FString DefaultURL;
	FString FilePath;
#if WITH_EDITOR
	TSharedPtr<class SChartWebBrowser> ChartWebBrowser;
#else
	TSharedPtr<class SWebBrowser> ChartWebBrowser;
#endif
	TSharedPtr<SOverlay> MyOverlay;
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
