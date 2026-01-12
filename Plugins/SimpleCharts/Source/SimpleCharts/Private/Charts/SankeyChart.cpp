// Copyright 2020 LHM. All Rights Reserved
#include "SankeyChart.h"

#define LOCTEXT_NAMESPACE "UMG"

USankeyChart::USankeyChart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TArray<FString> data = { "a","b","a1","a2","b1","c" };
	TArray<Flinks> links = {Flinks("a","a1",5.0f),Flinks("a","a2",3.0f),Flinks("b","b1",8.0f),Flinks("a","b1",3.0f), Flinks("b1","11",1.0f),Flinks("b1","c",2.0f) };
	Series = FSankeySeries(data, links,0.1f);

	series_json = MakeShareable(new FJsonObject);
}

void USankeyChart::OnLoadCompleted()
{
	Super::OnLoadCompleted();
	SetSeries(Series);
	SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
	//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
	UpdateChart();
}

void USankeyChart::SynchronizeProperties()
{
	Super::SynchronizeProperties();
		SetSeries(Series);
		SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
		//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
		SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
		UpdateChart();
}

#if WITH_EDITOR
const FText USankeyChart::GetPaletteCategory()
{
	return LOCTEXT("Chart", "SimpleChart");
}
#endif

void USankeyChart::SetLegend(bool show, Position position, Orient orient, FLinearColor color, int32 fontSize)
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
		//for (auto Serie : Series.data)
		//{
		//	legend_data.Add(MakeShareable(new FJsonValueString(Serie.Value)));
		//}
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

void USankeyChart::SetSeries(FSankeySeries SeriesData)
{
	Series = SeriesData;
	//Series setting
	TArray<TSharedPtr<FJsonValue>> tempdata;
	series_json.Reset();
	if (!series_json.IsValid())
	{
		series_json = MakeShareable(new FJsonObject);
	}
	for (auto d : Series.data)
	{
		TSharedPtr<FJsonObject> data_temp = MakeShareable(new FJsonObject);
		data_temp->SetStringField(TEXT("name"), d);
		tempdata.Add(MakeShareable(new FJsonValueObject(data_temp)));
	}
	TArray<TSharedPtr<FJsonValue>> templinks;
	for (auto d : Series.links)
	{
		TSharedPtr<FJsonObject> data_temp = MakeShareable(new FJsonObject);
		data_temp->SetStringField(TEXT("source"), d.source);
		data_temp->SetStringField(TEXT("target"), d.target);
		data_temp->SetNumberField(TEXT("value"), d.value);
		templinks.Add(MakeShareable(new FJsonValueObject(data_temp)));
	}
	
	//Set itemStyle
	if (Series.UseGradientColor)
	{
		series_json->SetObjectField(TEXT("itemStyle"), SetItemStyle(Series.LinearGradient));
	}

	series_json->SetObjectField(TEXT("label"), SetDataLabel(Series.DataLabel));
	series_json->SetArrayField(TEXT("links"), templinks);
	series_json->SetArrayField(TEXT("data"), tempdata);
	series_json->SetStringField(TEXT("type"), TEXT("sankey"));
	series_json->SetStringField(TEXT("left"), FString::SanitizeFloat(Series.left * 100) + "%");
	//series_json.Add(MakeShareable(new FJsonValueObject(temp)));

	if (!options_json.IsValid())
	{
		options_json = MakeShareable(new FJsonObject);
	}
	options_json->SetObjectField(TEXT("series"), series_json);
	SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	//UpdateChart();
}

#undef LOCTEXT_NAMESPACE
