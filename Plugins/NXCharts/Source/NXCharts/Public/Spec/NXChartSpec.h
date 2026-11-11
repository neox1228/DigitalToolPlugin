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

/** 标题组件配置 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartTitleSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FText Text;

	/** 留空时回落到引擎默认字体（打包后仍有效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title|Font")
	FSlateFontInfo Font;

	bool IsVisible() const { return !Text.IsEmpty(); }
};

/** 背景组件配置 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartBackgroundSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	FLinearColor Color = FLinearColor(0.02f, 0.03f, 0.05f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	bool bGradient = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background",
		meta = (EditCondition = "bGradient", EditConditionHides))
	FLinearColor ColorBottom = FLinearColor(0.01f, 0.02f, 0.04f, 1.0f);
};

/** 网格组件配置 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartGridSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FLinearColor Color = FLinearColor(0.16f, 0.19f, 0.24f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bDashed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid",
		meta = (EditCondition = "bDashed", EditConditionHides, ClampMin = "1.0"))
	float DashLength = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid",
		meta = (EditCondition = "bDashed", EditConditionHides, ClampMin = "0.0"))
	float GapLength = 4.0f;
};

/** 图例组件配置 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartLegendSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legend")
	bool bShow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legend",
		meta = (EditCondition = "bShow", EditConditionHides))
	EChartLegendPosition Position = EChartLegendPosition::Bottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legend|Font")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legend|Font")
	FLinearColor TextColor = FLinearColor(0.72f, 0.77f, 0.85f, 1.0f);
};

/** 跨组件共享的主题色（轴、刻度、标题、提示框） */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartColorsSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor AxisColor = FLinearColor(0.35f, 0.40f, 0.48f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor LabelColor = FLinearColor(0.72f, 0.77f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor TitleColor = FLinearColor(0.92f, 0.95f, 1.0f, 1.0f);
};


/** 注解类型 */
UENUM(BlueprintType)
enum class EChartAnnotationKind : uint8
{
	/** 水平线：Y = Value（阈值线） */
	HorizontalLine,
	/** 垂直线：X = Value（时间游标/分界） */
	VerticalLine,
	/** 水平色带：Value ~ RangeEnd（告警区间） */
	Band
};

/** 一条注解。放在 Spec 顶层，和系列平级 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartAnnotationSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	EChartAnnotationKind Kind = EChartAnnotationKind::HorizontalLine;

	/** HorizontalLine: Y 值；VerticalLine: X 值；Band: 起点 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	float Value = 0.0f;

	/** 仅 Band 使用：色带终点 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	float RangeEnd = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	FLinearColor Color = FLinearColor(0.95f, 0.45f, 0.35f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	float Thickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	bool bDashed = true;

	/** 线尾文字，留空不画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotation")
	FString Label;
};

/**
 * 一张图的完整配置 —— 这就是"图表资产"。
 * 可以序列化进 Widget BP，也可以在运行时整体替换；两者走同一条 ApplySpec 路径。
 */
USTRUCT(BlueprintType)
struct NXCHARTS_API FChartSpec
{
	GENERATED_BODY()

	/** 注解：阈值线 / 分界线 / 色带。空数组表示不画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Annotations")
	TArray<FChartAnnotationSpec> Annotations;
	/** 结构体版本。以后改字段时用它做升级，保证旧存档能读 */
	UPROPERTY()
	int32 Version = 1;

		// ---- 组件块：一个块对应一个绘制组件 ----

	/** 标题 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartTitleSpec Title;

	/** 背景 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartBackgroundSpec Background;

	/** 网格 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartGridSpec Grid;

	/** 图例 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartLegendSpec Legend;

	/** 跨组件共享色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	FChartColorsSpec Colors;

	// ---- 全局设置：不归属任何组件 ----

	/** 轴标签字体（轴与提示框共用）；留空回落引擎默认 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Layout")
	FSlateFontInfo AxisLabelFont;

	/** Y 轴标签带宽：>0 固定，缩放时绘图区不会左右抖；0 = 按实测自动 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Layout", meta = (ClampMin = "0.0"))
	float AxisLabelBandWidth = 0.0f;

	/** X 轴标签带宽：>0 固定；0 = 按实测自动 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Layout", meta = (ClampMin = "0.0"))
	float AxisLabelBandHeight = 0.0f;

	/** 控件最外圈内边距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Layout")
	FMargin Padding = FMargin(16.0f, 8.0f, 16.0f, 8.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart|Layout")
	bool bAntialias = true;

	/** 主题资产（预留，当前未参与渲染） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chart")
	TSoftObjectPtr<UNXChartTheme> Theme;

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