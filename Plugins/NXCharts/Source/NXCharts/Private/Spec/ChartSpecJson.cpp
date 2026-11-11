#include "Spec/ChartSpecJson.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Spec/NXChartSpec.h"
#include "Styling/CoreStyle.h"

namespace
{
	TSharedPtr<FJsonObject> Obj(const TSharedPtr<FJsonObject>& Parent, const FString& Key)
	{
		const TSharedPtr<FJsonObject>* Found = nullptr;
		return (Parent.IsValid() && Parent->TryGetObjectField(Key, Found) && Found) ? *Found : nullptr;
	}
	double N(const TSharedPtr<FJsonObject>& O, const FString& K, double D) { double V = D; return (O.IsValid() && O->TryGetNumberField(K, V)) ? V : D; }
	bool B(const TSharedPtr<FJsonObject>& O, const FString& K, bool D) { bool V = D; return (O.IsValid() && O->TryGetBoolField(K, V)) ? V : D; }
	FString S(const TSharedPtr<FJsonObject>& O, const FString& K, const FString& D = FString()) { FString V; return (O.IsValid() && O->TryGetStringField(K, V)) ? V : D; }
	FLinearColor Col(const TSharedPtr<FJsonObject>& O, const FString& K, const FLinearColor& D)
	{
		const FString T = S(O, K);
		return T.IsEmpty() ? D : FLinearColor(FColor::FromHex(T));
	}
	FSlateFontInfo Fnt(const TSharedPtr<FJsonObject>& O, const FString& K, float Size)
	{
		return FCoreStyle::GetDefaultFontStyle("Regular", (int32)N(Obj(O, K), "fontSize", Size));
	}
	template <typename T>
	void Pick(const FString& Text, T& Out, std::initializer_list<TPair<const TCHAR*, T>> Map)
	{
		for (const TPair<const TCHAR*, T>& Item : Map)
		{
			if (Text.Equals(Item.Key, ESearchCase::IgnoreCase)) { Out = Item.Value; return; }
		}
	}

	void ParseAxis(const TSharedPtr<FJsonObject>& O, FChartAxisSpec& Axis)
	{
		if (!O.IsValid()) { return; }
		Axis.AxisTitle = FText::FromString(S(O, "title", Axis.AxisTitle.ToString()));
		Axis.Unit = S(O, "unit", Axis.Unit);
		Axis.TickSuffix = S(O, "tickSuffix", Axis.TickSuffix);
		Axis.DesiredTickCount = (int32)N(O, "desiredTicks", Axis.DesiredTickCount);
		Axis.bShowGrid = B(O, "showGrid", Axis.bShowGrid);
		Axis.bShowLabels = B(O, "showLabels", Axis.bShowLabels);
		Axis.bUseHardLimits = B(O, "hardLimits", Axis.bUseHardLimits);
		Axis.Padding = (float)N(O, "padding", Axis.Padding);

		if (const TSharedPtr<FJsonObject> LB = Obj(O, "lowerBound"))
		{
			Axis.LowerBound.Mode = EChartAxisBoundMode::Limit;
			Axis.LowerBound.Value = (float)N(LB, "value", 0.0);
		}
		if (const TSharedPtr<FJsonObject> UB = Obj(O, "upperBound"))
		{
			Axis.UpperBound.Mode = EChartAxisBoundMode::Limit;
			Axis.UpperBound.Value = (float)N(UB, "value", 0.0);
		}
		Axis.HardLimits = FVector2D(N(O, "min", Axis.HardLimits.X), N(O, "max", Axis.HardLimits.Y));
		if (B(O, "manual", false))
		{
			Axis.DomainMode = EChartDomainMode::Manual;
			Axis.ManualDomain = Axis.HardLimits;
		}
		Pick(S(O, "format"), Axis.Format, {
			{ TEXT("auto"), EChartTickFormat::Auto }, { TEXT("integer"), EChartTickFormat::Integer },
			{ TEXT("oneDecimal"), EChartTickFormat::OneDecimal }, { TEXT("twoDecimals"), EChartTickFormat::TwoDecimals },
			{ TEXT("percent"), EChartTickFormat::Percent }, { TEXT("scientific"), EChartTickFormat::Scientific } });
	}
}

bool FChartSpecJson::LoadFromFile(const FString& FullPath, FChartSpec& OutSpec, FString& OutError)
{
	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *FullPath))
	{
		OutError = TEXT("文件读取失败");
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = TEXT("JSON 解析失败（检查逗号与括号）");
		return false;
	}

	FChartSpec Spec;

	if (const TSharedPtr<FJsonObject> O = Obj(Root, "title"))
	{
		Spec.Title.Text = FText::FromString(S(O, "text", Spec.Title.Text.ToString()));
		Spec.Title.Font = Fnt(O, "font", 14.0f);
	}
	if (const TSharedPtr<FJsonObject> O = Obj(Root, "background"))
	{
		Spec.Background.Color = Col(O, "color", Spec.Background.Color);
		Spec.Background.bGradient = B(O, "gradient", Spec.Background.bGradient);
		Spec.Background.ColorBottom = Col(O, "colorBottom", Spec.Background.ColorBottom);
	}
	if (const TSharedPtr<FJsonObject> O = Obj(Root, "grid"))
	{
		Spec.Grid.Color = Col(O, "color", Spec.Grid.Color);
		Spec.Grid.bDashed = B(O, "dashed", Spec.Grid.bDashed);
		Spec.Grid.DashLength = (float)N(O, "dashLength", Spec.Grid.DashLength);
		Spec.Grid.GapLength = (float)N(O, "gapLength", Spec.Grid.GapLength);
	}
	if (const TSharedPtr<FJsonObject> O = Obj(Root, "legend"))
	{
		Spec.Legend.bShow = B(O, "show", Spec.Legend.bShow);
		Spec.Legend.TextColor = Col(O, "textColor", Spec.Legend.TextColor);
		Spec.Legend.Font = Fnt(O, "font", 9.0f);
		Pick(S(O, "position"), Spec.Legend.Position, {
			{ TEXT("top"), EChartLegendPosition::Top }, { TEXT("bottom"), EChartLegendPosition::Bottom },
			{ TEXT("right"), EChartLegendPosition::Right } });
	}
	if (const TSharedPtr<FJsonObject> O = Obj(Root, "colors"))
	{
		Spec.Colors.AxisColor = Col(O, "axis", Spec.Colors.AxisColor);
		Spec.Colors.LabelColor = Col(O, "label", Spec.Colors.LabelColor);
		Spec.Colors.TitleColor = Col(O, "title", Spec.Colors.TitleColor);
	}

	Spec.AxisLabelBandWidth = (float)N(Root, "axisLabelBandWidth", Spec.AxisLabelBandWidth);
	Spec.AxisLabelBandHeight = (float)N(Root, "axisLabelBandHeight", Spec.AxisLabelBandHeight);

	ParseAxis(Obj(Root, "axisX"), Spec.AxisX);
	ParseAxis(Obj(Root, "axisY"), Spec.AxisY);

	const TArray<TSharedPtr<FJsonValue>>* SeriesArray = nullptr;
	if (Root->TryGetArrayField(TEXT("series"), SeriesArray) && SeriesArray)
	{
		for (const TSharedPtr<FJsonValue>& Value : *SeriesArray)
		{
			const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!O.IsValid()) { continue; }

			FChartSeriesSpec Series;
			Series.SeriesKey = FName(*S(O, "key"));
			if (Series.SeriesKey.IsNone()) { continue; }
			Series.DisplayName = FText::FromString(S(O, "name"));
			Series.Style.Color = Col(O, "color", Series.Style.Color);
			Series.Style.LineThickness = (float)N(O, "lineWidth", Series.Style.LineThickness);
			Series.Style.bFillArea = B(O, "fill", Series.Style.bFillArea);
			Series.Style.bGlow = B(O, "glow", Series.Style.bGlow);
			Series.bVisible = B(O, "visible", Series.bVisible);
			Pick(S(O, "curve"), Series.Style.Curve, {
				{ TEXT("linear"), EChartCurveType::Linear }, { TEXT("smooth"), EChartCurveType::Smooth },
				{ TEXT("step"), EChartCurveType::Step } });
			Pick(S(O, "type"), Series.Kind, {
				{ TEXT("line"), EChartSeriesKind::Line }, { TEXT("bar"), EChartSeriesKind::Bar },
				{ TEXT("scatter"), EChartSeriesKind::Scatter }, { TEXT("area"), EChartSeriesKind::Area } });
			Spec.Series.Add(Series);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* AnnotationArray = nullptr;
	if (Root->TryGetArrayField(TEXT("annotations"), AnnotationArray) && AnnotationArray)
	{
		for (const TSharedPtr<FJsonValue>& Value : *AnnotationArray)
		{
			const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!O.IsValid()) { continue; }

			FChartAnnotationSpec Annotation;
			Pick(S(O, "kind", TEXT("horizontalLine")), Annotation.Kind, {
				{ TEXT("horizontalLine"), EChartAnnotationKind::HorizontalLine },
				{ TEXT("verticalLine"), EChartAnnotationKind::VerticalLine },
				{ TEXT("band"), EChartAnnotationKind::Band } });
			Annotation.Value = (float)N(O, "value", 0.0);
			Annotation.RangeEnd = (float)N(O, "rangeEnd", 0.0);
			Annotation.Color = Col(O, "color", Annotation.Color);
			Annotation.Thickness = (float)N(O, "thickness", Annotation.Thickness);
			Annotation.bDashed = B(O, "dashed", Annotation.bDashed);
			Annotation.Label = S(O, "label");
			Spec.Annotations.Add(Annotation);
		}
	}

	OutSpec = Spec;
	return true;
}