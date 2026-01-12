// Copyright 2020 LHM. All Rights Reserved
#include "RadarChart.h"

#define LOCTEXT_NAMESPACE "UMG"

URadarChart::URadarChart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Series.name = "default";
	Series.data.Add(FData("1", { 6, 2, 4,3, 5, 7 }));
	Series.data.Add(FData("2", { 3, 5, 7,3, 5, 7 }));
	Series.data.Add(FData("3", { 4, 2, 3 ,6, 2, 4 }));
	Series.data.Add(FData("4", { 6, 2, 4,3, 5, 7 }));
	Series.data.Add(FData("5", { 3, 5, 7,4, 2, 3 }));
	Series.data.Add(FData("6", { 4, 2, 3 ,4, 2, 3 }));
	Radar.indicator.Add(FIndicator("1",10));
	Radar.indicator.Add(FIndicator("2", 10));
	Radar.indicator.Add(FIndicator("3", 10));
	Radar.indicator.Add(FIndicator("4", 10));
	Radar.indicator.Add(FIndicator("5", 10));
	Radar.indicator.Add(FIndicator("6", 10));

	Radar_json = MakeShareable(new FJsonObject);
}

void URadarChart::OnLoadCompleted()
{
	Super::OnLoadCompleted();
	SetSeries(Series);
	SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
	//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	SetTooltipStyle(tooltip_trigger, tooltip_type, text_color, tip_backgroundColor);
	SetRadar(Radar);
	UpdateChart();
}

void URadarChart::SynchronizeProperties()
{
	Super::SynchronizeProperties();
		SetSeries(Series);
		SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
		//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
		SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
		SetRadar(Radar);
		UpdateChart();
}

#if WITH_EDITOR
const FText URadarChart::GetPaletteCategory()
{
	return LOCTEXT("Chart", "SimpleChart");
}
#endif

void URadarChart::SetLegend(bool show, Position position, Orient orient, FLinearColor  color, int32 fontSize)
{
	legend_show = show;
	legend_position = position;
	legend_orient = orient;
	legend_color = color.ToFColor(true);
	legend_fontSize = fontSize;
	if (!legend_json.IsValid())
	{
		legend_json = MakeShareable(new FJsonObject);
	}
	//legend setting
	legend_json->SetBoolField(TEXT("show"), legend_show);
	if (legend_show)
	{
		switch (legend_position)
		{
		case Position::center:
			legend_json->SetStringField(TEXT("x"), "center");
			break;
		case Position::left:
			legend_json->SetStringField(TEXT("x"), "left");
			break;
		case Position::right:
			legend_json->SetStringField(TEXT("x"), "right");
			break;
		default:
			break;
		}
		switch (legend_orient)
		{
		case Orient::horizontal:
			legend_json->SetStringField(TEXT("orient"), "horizontal");
			break;
		case Orient::vertical:
			legend_json->SetStringField(TEXT("orient"), "vertical");
			break;
		default:
			break;
		}
		TArray<TSharedPtr<FJsonValue>> legend_data;
		for (auto d : Series.data)
		{
			legend_data.Add(MakeShareable(new FJsonValueString(d.name)));
		}
		legend_json->SetArrayField(TEXT("data"), legend_data);
		TSharedPtr<FJsonObject> legend_textStyle = MakeShareable(new FJsonObject);;
		legend_textStyle->SetNumberField(TEXT("fontSize"), legend_fontSize);
		legend_textStyle->SetStringField(TEXT("color"), ColorToRGBA(legend_color));
		legend_json->SetObjectField(TEXT("textStyle"), legend_textStyle);
	}
	if (!options_json.IsValid())
	{
		options_json = MakeShareable(new FJsonObject);
	}
	options_json->SetObjectField(TEXT("legend"), legend_json);
	//UpdateChart();
}

void URadarChart::SetSeries(FRadarSeries SeriesData)
{
	Series = SeriesData;
	//Series setting
	series_json.Reset();
	TSharedPtr<FJsonObject> temp = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> tempdata;
	for (auto d : Series.data)
	{
		TSharedPtr<FJsonObject> data_temp = MakeShareable(new FJsonObject);
		data_temp->SetStringField(TEXT("name"), d.name);
		TArray<TSharedPtr<FJsonValue>> value_array;
		for (auto v:d.value)
		{
			value_array.Add(MakeShareable(new FJsonValueNumber(v)));
;		}
		data_temp->SetArrayField(TEXT("value"), value_array);
		tempdata.Add(MakeShareable(new FJsonValueObject(data_temp)));
	}

	//Set itemStyle
	if (Series.UseGradientColor)
	{
		temp->SetObjectField(TEXT("itemStyle"), SetItemStyle(Series.LinearGradient));
	}

	temp->SetStringField(TEXT("name"), Series.name);
	temp->SetArrayField(TEXT("data"), tempdata);
	temp->SetStringField(TEXT("type"), TEXT("radar"));
	series_json.Add(MakeShareable(new FJsonValueObject(temp)));
	if (!options_json.IsValid())
	{
		options_json = MakeShareable(new FJsonObject);
	}
	options_json->SetArrayField(TEXT("series"), series_json);
	SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	//UpdateChart();
}

void URadarChart::SetRadar(FRadar RadarData)
{
	Radar = RadarData;
	//textStyle
	TSharedPtr<FJsonObject> textStyle = MakeShareable(new FJsonObject);
	FString color = ColorToRGBA(Radar.color);
	textStyle->SetStringField(TEXT("color"), color);
	FString backgroundColor = ColorToRGBA(Radar.backgroundColor);
	textStyle->SetStringField(TEXT("backgroundColor"), backgroundColor);
	textStyle->SetNumberField(TEXT("borderRadius"),3);
	TArray<TSharedPtr<FJsonValue>> padding_array;
	padding_array.Add(MakeShareable(new FJsonValueNumber(3)));
	padding_array.Add(MakeShareable(new FJsonValueNumber(5)));
	textStyle->SetArrayField(TEXT("padding"), padding_array);

	//indicator
	TArray<TSharedPtr<FJsonValue>> indicator;
	for (auto i: Radar.indicator)
	{
		TSharedPtr<FJsonObject> temp = MakeShareable(new FJsonObject);
		temp->SetStringField(TEXT("name"), i.name);
		temp->SetNumberField(TEXT("max"), i.max);
		indicator.Add(MakeShareable(new FJsonValueObject(temp)));
	}

	if (!Radar_json.IsValid())
	{
		Radar_json = MakeShareable(new FJsonObject);
	}
	Radar_json->SetStringField(TEXT("radius"), FString::SanitizeFloat(Radar.radius * 100) + "%");
	if (Radar.circle)
	{
		Radar_json->SetStringField(TEXT("shape"), TEXT("circle"));
	}
	Radar_json->SetObjectField(TEXT("name"), textStyle);
	Radar_json->SetArrayField(TEXT("indicator"), indicator);
	if (!options_json.IsValid())
	{
		options_json = MakeShareable(new FJsonObject);
	}
	options_json->SetObjectField(TEXT("radar"), Radar_json);
}
#undef LOCTEXT_NAMESPACE
