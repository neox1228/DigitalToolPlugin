// Copyright 2020 LHM. All Rights Reserved
#include "ScatterChart.h"

#define LOCTEXT_NAMESPACE "UMG"

UScatterChart::UScatterChart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AxisData.Empty();
	FScatterSeries Default;
	Default.data.Add(FVector2D(10.0f, 8.04f));
	Default.data.Add(FVector2D(8.0f, 6.95f));
	Default.data.Add(FVector2D(12.0f, 10.84f));
	Default.name = "default";
	Series.Add(Default);
}

void UScatterChart::OnLoadCompleted()
{
	Super::OnLoadCompleted();
	SetSeries(Series);
	SetXAxisStyle(XaxisLabel, XaxisLine, XsplitLine);
	SetYAxisStyle(YaxisLabel, YaxisLine, YsplitLine);
	//SetAxis(IsXAxis, Axis);
	SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
	//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	SetTooltipStyle(tooltip_trigger, tooltip_type, text_color, tip_backgroundColor);
	UpdateChart();
}

void UScatterChart::SynchronizeProperties()
{
	Super::SynchronizeProperties();
		SetSeries(Series);
		SetXAxisStyle(XaxisLabel, XaxisLine, XsplitLine);
		SetYAxisStyle(YaxisLabel, YaxisLine, YsplitLine);
		//SetAxis(IsXAxis, Axis);
		SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
		//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
		SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
		UpdateChart();
}

#if WITH_EDITOR
const FText UScatterChart::GetPaletteCategory()
{
	return LOCTEXT("LineChart", "SimpleChart");
}
#endif

void UScatterChart::SetLegend(bool show, Position position, Orient orient, FLinearColor color, int32 fontSize)
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
		for (auto Serie : Series)
		{
			legend_data.Add(MakeShareable(new FJsonValueString(Serie.name)));
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

void UScatterChart::SetSeries(TArray<FScatterSeries> SeriesData)
{
	Series = SeriesData;
	//Series setting
	series_json.Reset();
	for (auto Serie : Series)
	{
		TSharedPtr<FJsonObject> temp = MakeShareable(new FJsonObject);
		temp->SetStringField(TEXT("name"), Serie.name);
		TArray<TSharedPtr<FJsonValue>> tempdata;
		for (auto d : Serie.data)
		{
			TArray<TSharedPtr<FJsonValue>> data_temp;
			data_temp.Add(MakeShareable(new FJsonValueNumber(d[0])));
			data_temp.Add(MakeShareable(new FJsonValueNumber(d[1])));
			tempdata.Add(MakeShareable(new FJsonValueArray(data_temp)));
		}
		temp->SetArrayField(TEXT("data"), tempdata);
		temp->SetStringField(TEXT("type"), TEXT("scatter"));
		temp->SetNumberField(TEXT("symbolSize"), Serie.symbolSize);

		//Set Serie Label
		temp->SetObjectField(TEXT("label"), SetDataLabel(Serie.DataLabel));

		//Set itemStyle
		if (Serie.UseGradientColor)
		{
			temp->SetObjectField(TEXT("itemStyle"), SetItemStyle(Serie.LinearGradient));
		}

		series_json.Add(MakeShareable(new FJsonValueObject(temp)));
	}
	if (!options_json.IsValid())
	{
		options_json = MakeShareable(new FJsonObject);
	}
	options_json->SetArrayField(TEXT("series"), series_json);
	SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	//UpdateChart();
}
#undef LOCTEXT_NAMESPACE
