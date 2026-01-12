// Copyright 2020 LHM. All Rights Reserved
#include "DoughnutChart.h"

#define LOCTEXT_NAMESPACE "UMG"

UDoughnutChart::UDoughnutChart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Series.data.Add("key01",3.0f);
	Series.data.Add("key02",2.0f);
	Series.data.Add("key03",5.0f);
	Series.name = "default";
}

void UDoughnutChart::OnLoadCompleted()
{
	Super::OnLoadCompleted();
	SetSeries(Series);
	SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
	//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
	UpdateChart();
}

void UDoughnutChart::SynchronizeProperties()
{
	Super::SynchronizeProperties();
		SetSeries(Series);
		SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
		//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
		SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
		UpdateChart();
}

#if WITH_EDITOR
const FText UDoughnutChart::GetPaletteCategory()
{
	return LOCTEXT("LineChart", "SimpleChart");
}
#endif

void UDoughnutChart::SetLegend(bool show, Position position, Orient orient, FLinearColor color, int32 fontSize)
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
		for (auto Serie : Series.data)
		{
			legend_data.Add(MakeShareable(new FJsonValueString(Serie.Key)));
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

void UDoughnutChart::SetSeries(FDoughnutSeries SeriesData)
{
	Series = SeriesData;
	//Series setting
	series_json.Reset();
	TSharedPtr<FJsonObject> temp = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> tempdata;
	TArray<TSharedPtr<FJsonValue>> radius;
	radius.Add(MakeShareable(new FJsonValueString(FString::SanitizeFloat(Series.inside_radius * 100)+"%")));
	radius.Add(MakeShareable(new FJsonValueString(FString::SanitizeFloat(Series.outside_radius * 100) + "%")));
	for (auto d : Series.data)
	{
		TSharedPtr<FJsonObject> data_temp = MakeShareable(new FJsonObject);
		data_temp->SetNumberField(TEXT("value"), d.Value);
		data_temp->SetStringField(TEXT("name"), d.Key);
		tempdata.Add(MakeShareable(new FJsonValueObject(data_temp)));
	}

	TSharedPtr<FJsonObject> label = MakeShareable(new FJsonObject);
	label->SetBoolField(TEXT("show"), Series.label_show);
	TSharedPtr<FJsonObject> labelLine = MakeShareable(new FJsonObject);
	labelLine->SetBoolField(TEXT("show"), Series.labelLine_show);

	//Set itemStyle
	if (Series.UseGradientColor)
	{
		temp->SetObjectField(TEXT("itemStyle"), SetItemStyle(Series.LinearGradient));
	}

	temp->SetObjectField(TEXT("label"), label);
	temp->SetObjectField(TEXT("labelLine"), labelLine);
	temp->SetArrayField(TEXT("data"), tempdata);
	temp->SetStringField(TEXT("type"), TEXT("pie"));
	temp->SetArrayField(TEXT("radius"), radius);
	series_json.Add(MakeShareable(new FJsonValueObject(temp)));
	if (!options_json.IsValid())
	{
		options_json = MakeShareable(new FJsonObject);
	}
	options_json->SetArrayField(TEXT("series"), series_json);
	SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	//UpdateChart();
}
#undef LOCTEXT_NAMESPACE
