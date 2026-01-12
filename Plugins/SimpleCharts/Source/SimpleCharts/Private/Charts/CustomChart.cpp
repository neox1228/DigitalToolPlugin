// Copyright 2020 LHM. All Rights Reserved
#include "CustomChart.h"
#include "Runtime/Json/Public/Serialization/JsonReader.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "UMG"

UCustomChart::UCustomChart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if PLATFORM_ANDROID
	extern FString GExternalFilePath;
	DefaultURL = "file:///" + GExternalFilePath / "Source/ThirdParty/html/index.html#/Chart";
#else
	FilePath = FPaths::ConvertRelativePathToFull(FPaths::EnginePluginsDir() + "Marketplace/SimpleCharts/Source/ThirdParty/html/");
	if (!FPaths::DirectoryExists(FilePath))
	{
		FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir() + "SimpleCharts/Source/ThirdParty/html/");
		if (!FPaths::DirectoryExists(FilePath))
		{
			FilePath = FPaths::ConvertRelativePathToFull(FPaths::EnterprisePluginsDir() + "Marketplace/SimpleCharts/Source/ThirdParty/html/");
		}
	}
	DefaultURL = "file:///" + FilePath + "index.html#/Chart";
#endif
	//options_json = MakeShareable(new FJsonObject);
	//Option = "{\"xAxis\": {\"data\" : [\"Mon\", \"Tue\", \"Wed\", \"Thu\", \"Fri\",\"Sat\", \"Sun\"]},\"yAxis\" : {\"type\": \"value\"},\"series\" : [{\"data\": [820, 932, 901, 934, 1290, 1330, 1320],\"type\" : \"line\"}]}";
}

FString UCustomChart::ThemeToString()
{
	FString Theme_str;
	switch (Theme)
	{
	default:
		break;
	case ChartTheme::azul:
		Theme_str = "azul";
		break;
	case ChartTheme::beeinspired:
		Theme_str = "bee-inspired";
		break;
	case ChartTheme::blue:
		Theme_str = "blue";
		break;
	case ChartTheme::caravan:
		Theme_str = "caravan";
		break;
	case ChartTheme::carp:
		Theme_str = "carp";
		break;
	case ChartTheme::cool:
		Theme_str = "cool";
		break;
	case ChartTheme::dark:
		Theme_str = "dark";
		break;
	case ChartTheme::darkblue:
		Theme_str = "dark-blue";
		break;
	case ChartTheme::darkbold:
		Theme_str = "dark-bold";
		break;
	case ChartTheme::darkdigerati:
		Theme_str = "dark-digerati";
		break;
	case ChartTheme::darkfreshcut:
		Theme_str = "dark-fresh-cut";
		break;
	case ChartTheme::darkmushroom:
		Theme_str = "dark-mushroom";
		break;
	case ChartTheme::eduardo:
		Theme_str = "eduardo";
		break;
	case ChartTheme::forest:
		Theme_str = "forest";
		break;
	case ChartTheme::freshcut:
		Theme_str = "fresh-cut";
		break;
	case ChartTheme::fruit:
		Theme_str = "fruit";
		break;
	case ChartTheme::gray:
		Theme_str = "gray";
		break;
	case ChartTheme::green:
		Theme_str = "green";
		break;
	case ChartTheme::helianthus:
		Theme_str = "helianthus";
		break;
	case ChartTheme::infographic:
		Theme_str = "infographic";
		break;
	case ChartTheme::inspired:
		Theme_str = "inspired";
		break;
	case ChartTheme::jazz:
		Theme_str = "jazz";
		break;
	case ChartTheme::london:
		Theme_str = "london";
		break;
	case ChartTheme::macarons:
		Theme_str = "macarons";
		break;
	case ChartTheme::macarons2:
		Theme_str = "macarons2";
		break;
	case ChartTheme::mint:
		Theme_str = "mint";
		break;
	case ChartTheme::red:
		Theme_str = "red";
		break;
	case ChartTheme::redvelvet:
		Theme_str = "red-velvet";
		break;
	case ChartTheme::roma:
		Theme_str = "roma";
		break;
	case ChartTheme::royal:
		Theme_str = "royal";
		break;
	case ChartTheme::sakura:
		Theme_str = "sakura";
		break;
	case ChartTheme::shine:
		Theme_str = "shine";
		break;
	case ChartTheme::techblue:
		Theme_str = "tech-blue";
		break;
	case ChartTheme::vintage:
		Theme_str = "vintage";
		break;
	}
	return Theme_str;
}

void UCustomChart::OnLoadCompleted()
{
#if WITH_EDITOR
	if (ChartWebBrowser.IsValid() && MyOverlay.IsValid() && MyOverlay->GetChildren())
	{
		MyOverlay->GetChildren()->GetChildAt(0).Get().SetVisibility(EVisibility::Hidden);
	}
#else 
	
#endif
	SetOption(Option);
}

void UCustomChart::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	SetOption(Option);
}

void UCustomChart::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyOverlay.Reset();
	ChartWebBrowser.Reset();
}

#if WITH_EDITOR
const FText UCustomChart::GetPaletteCategory()
{
	return LOCTEXT("Chart", "SimpleChart");
}
#endif

void UCustomChart::SetTheme(ChartTheme ChartTheme)
{
	Theme = ChartTheme;
	SetOption(Option);
}

void UCustomChart::SetOption(FString InOption, FString OtherCode)
{
	Option = InOption;
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Option);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		SetOptionByJson(Option);
	}
	else {
		SetOptionByObject(Option, OtherCode);
	}
}

void UCustomChart::SetOptionByJson(FString json)
{
	//json  string  send to web
	FString t = "\"";
	if (ChartWebBrowser.IsValid())
	{
		ChartWebBrowser->ExecuteJavascript("vue.SetMyOptions(" + json + "," + t + ThemeToString() + t + ")");
	}
	
}

void UCustomChart::SetOptionByObject(FString Object, FString OtherCode)
{
	//Object to string and send to web
	FString t = "\"";
	FString Str01 = OtherCode + ";";
	FString Temp = Object.Replace(TEXT("echarts.graphic.LinearGradient"), TEXT("vue.$echarts.graphic.LinearGradient"));
	Str01 += "var object = " + Temp + ";";

	FString useDirtyRectStr;
	if (useDirtyRect)
	{
		useDirtyRectStr = "true";
	}
	else {
		useDirtyRectStr = "false";
	}

	FString Str02 = Str01 + "vue.SetMyOptions(object ," + t + ThemeToString() + t + "," + useDirtyRectStr + ")";
	if (ChartWebBrowser.IsValid())
	{
		ChartWebBrowser->ExecuteJavascript(Str02);
	}
}

TSharedRef<SWidget> UCustomChart::RebuildWidget()
{
#if WITH_EDITOR
	ChartWebBrowser = SNew(SChartWebBrowser)
		.InitialURL(DefaultURL)
		.SupportsTransparency(true)
		.OnLoadCompleted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, OnLoadCompleted));

	MyOverlay = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SCircularThrobber)
		]
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			ChartWebBrowser.ToSharedRef()
		]
		];
#else
	ChartWebBrowser = SNew(SWebBrowser)
		.InitialURL(DefaultURL)
		.ShowControls(false)
		.SupportsTransparency(true)
		.OnLoadCompleted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, OnLoadCompleted));

	MyOverlay = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			ChartWebBrowser.ToSharedRef()
		]
		];
#endif
	return MyOverlay.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE