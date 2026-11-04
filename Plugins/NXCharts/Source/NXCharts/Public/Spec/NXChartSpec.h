// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartSpec.h
 * @brief   配置根对象：一张图的全部外观与结构，纯值语义。
 *
 * 这是整个插件的「唯一事实源」：
 *  - 可序列化（Widget BP / SaveGame / DataTable / 后端下发）
 *  - 可整体比较（重建判定与增量协调依赖它）
 *  - 可在运行时整体替换（SetChartSpec），不需要改 UMG 资产
 *
 * 关键点：Spec 里不存数据点。数据走 FChartSeriesDataPool，按 SeriesKey 对齐。
 */

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Core/NXChartTheme.h"
#include "Spec/NXChartSpecTypes.h"
#include "NXChartSpec.generated.h"

/** 图例位置 */
UENUM(BlueprintType)
enum class EChartLegendPosition : uint8
{
	Top,
	Bottom,
	Right
};

/** 图表级外观配置 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartOptionsSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	FText Title;

	/** 字体留空时回落到引擎默认字体（打包后仍有效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Font")
	FSlateFontInfo TitleFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Font")
	FSlateFontInfo AxisLabelFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Font")
	FSlateFontInfo LegendFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	TSoftObjectPtr<UNXChartTheme> Theme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	FLinearColor BackgroundColor = FLinearColor(0.02f, 0.03f, 0.05f, 1.0f);

	/** 背景垂直渐变：从 BackgroundColor 渐变到这里 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bGradientBackground = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options",
		meta = (EditCondition = "bGradientBackground", EditConditionHides))
	FLinearColor BackgroundColorBottom = FLinearColor(0.01f, 0.02f, 0.04f, 1.0f);

	// ---- 主题色。放在 Spec 里而不是渲染器里，才能做出不同风格的图表

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Colors")
	FLinearColor GridColor = FLinearColor(0.16f, 0.19f, 0.24f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Colors")
	FLinearColor AxisColor = FLinearColor(0.35f, 0.40f, 0.48f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Colors")
	FLinearColor LabelColor = FLinearColor(0.72f, 0.77f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Colors")
	FLinearColor TitleColor = FLinearColor(0.92f, 0.95f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Colors")
	FLinearColor LegendTextColor = FLinearColor(0.72f, 0.77f, 0.85f, 1.0f);

	// ---- 网格样式

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Grid")
	bool bDashedGrid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Grid",
		meta = (EditCondition = "bDashedGrid", EditConditionHides, ClampMin = "1.0"))
	float GridDashLength = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options|Grid",
		meta = (EditCondition = "bDashedGrid", EditConditionHides, ClampMin = "0.0"))
	float GridGapLength = 4.0f;

	/** 控件最外圈的内边距，布局会在此基础上再按标签实测尺寸加码 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	FMargin Padding = FMargin(16.0f, 8.0f, 16.0f, 8.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bAntialias = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bShowLegend = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options", meta = (EditCondition = "bShowLegend", EditConditionHides))
	EChartLegendPosition LegendPosition = EChartLegendPosition::Bottom;
};

/**
 * 一张图的完整配置 —— 这就是"图表资产"。
 * 可以序列化进 Widget BP，也可以在运行时整体替换；两者走同一条 ApplySpec 路径。
 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartSpec
{
	GENERATED_BODY()

	/** 结构体版本。以后改字段时用它做升级，保证旧存档能读 */
	UPROPERTY()
	int32 Version = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartOptionsSpec Options;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartAxisSpec AxisX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartAxisSpec AxisY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	TArray<FChartSeriesSpec> Series;

	// ---------------------------------------------------------------- 查询辅助

	const FChartSeriesSpec* FindSeries(FName Key) const
	{
		for (const FChartSeriesSpec& Item : Series)
		{
			if (Item.SeriesKey == Key) return &Item;
		}
		return nullptr;
	}

	FChartSeriesSpec* FindSeriesMutable(FName Key)
	{
		for (FChartSeriesSpec& Item : Series)
		{
			if (Item.SeriesKey == Key) return &Item;
		}
		return nullptr;
	}

	int32 IndexOfSeries(FName Key) const
	{
		for (int32 Index = 0; Index < Series.Num(); ++Index)
		{
			if (Series[Index].SeriesKey == Key) return Index;
		}
		return INDEX_NONE;
	}

	int32 CountVisibleSeries() const
	{
		int32 Count = 0;
		for (const FChartSeriesSpec& Item : Series)
		{
			if (Item.bVisible) ++Count;
		}
		return Count;
	}
};

/** 单条诊断信息 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartDiagnostic
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Diagnostics")
	FString Message;

	FChartDiagnostic() {}
	explicit FChartDiagnostic(const FString& InMessage) : Message(InMessage) {}
};

/**
 * 配置应用的结果报告。
 * 设计原则：配置非法不抛异常、不中断流程，而是丢弃非法项并留下可被蓝图读取的诊断。
 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartDiagnostics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Diagnostics")
	TArray<FChartDiagnostic> Items;

	void Reset() { Items.Reset(); }

	void Add(const FString& Message) { Items.Emplace(Message); }

	bool HasAny() const { return Items.Num() > 0; }

	FString ToString() const
	{
		FString Result;
		for (const FChartDiagnostic& Item : Items)
		{
			if (!Result.IsEmpty()) Result += TEXT("\n");
			Result += Item.Message;
		}
		return Result;
	}
};