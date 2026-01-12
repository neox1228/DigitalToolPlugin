// Copyright 2020 LHM. All Rights Reserved
#include "PieChart.h"

#define LOCTEXT_NAMESPACE "UMG"

UPieChart::UPieChart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Series.data.Add("key01",3.0f);
	Series.data.Add("key02",2.0f);
	Series.data.Add("key03",5.0f);
	Series.name = "default";
}

void UPieChart::OnLoadCompleted()
{
	Super::OnLoadCompleted();
	SetSeries(Series);
	SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
	//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
	SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
	UpdateChart();
}

void UPieChart::SynchronizeProperties()
{
	Super::SynchronizeProperties();
		SetSeries(Series);
		SetTitle(title_text, title_show, title_position, title_fontSize, title_color);
		//SetLegend(legend_show, legend_position, legend_orient, legend_color, legend_fontSize);
		SetTooltipStyle(tooltip_trigger, tooltip_type,text_color,tip_backgroundColor);
		UpdateChart();
}

#if WITH_EDITOR
const FText UPieChart::GetPaletteCategory()
{
	return LOCTEXT("LineChart", "SimpleChart");
}
#endif

//void UPieChart::UpdateChart()
//{
//	if (WebObject)
//	{
//		//json to string and send to web
//		FString JsonStr;
//		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
//		FJsonSerializer::Serialize(options_json.ToSharedRef(), Writer);
//		WebObject->ExecuteJS("vue.SetMyOptions(" + JsonStr + ")");
//		//UE_LOG(LogTemp, Warning, TEXT("%s"), *JsonStr);
//	}
//}

void UPieChart::SetLegend(bool show, Position position, Orient orient, FLinearColor  color, int32 fontSize)
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

void UPieChart::SetSeries(FPieSeries SeriesData)
{
	Series = SeriesData;
	//Series setting
	series_json.Reset();
	TSharedPtr<FJsonObject> temp = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> tempdata;
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

	if (Series.RoseType)
	{
		temp->SetStringField(TEXT("roseType"), TEXT("radius"));
	}

	//Set itemStyle
	if (Series.UseGradientColor)
	{
		temp->SetObjectField(TEXT("itemStyle"), SetItemStyle(Series.LinearGradient));
	}

	temp->SetObjectField(TEXT("label"), label);
	temp->SetObjectField(TEXT("labelLine"), labelLine);
	temp->SetArrayField(TEXT("data"), tempdata);
	temp->SetStringField(TEXT("type"), TEXT("pie"));
	temp->SetStringField(TEXT("radius"), FString::SanitizeFloat(Series.radius * 100) + "%");
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
