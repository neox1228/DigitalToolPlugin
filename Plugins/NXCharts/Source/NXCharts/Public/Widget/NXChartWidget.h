// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartWidget.h
 * @brief   UMG / 蓝图侧的唯一入口。
 *
 * 职责边界：
 *  - 持有 Spec（配置）、DataPool（数据）、CachedFrame（帧缓存）
 *  - 对外暴露运行时 API：增删系列、灌数据、改样式、域覆盖
 *  - 决定"什么时候重建帧"（GetFrame 里做版本比较，绘制期不重算）
 *
 * 不做：任何绘制、任何布局计算、任何坐标映射——那些在 Frame / PaintContext 里。
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Data/ChartSeriesBuffer.h"
#include "Frame/ChartFrame.h"
#include "Spec/NXChartSpec.h"
#include "NXChartWidget.generated.h"

class SChartCanvas;

UCLASS()
class NXCHARTS_API UNXChartWidget : public UWidget
{
	GENERATED_BODY()

public:
	// ---------------------------------------------------------------- 设计期配置

	/** 图表配置。设计期可直接编辑，运行时可被 SetChartSpec 覆盖 */
		/** 配置文件路径，相对 Content 目录，例如 "NXCharts/Demo.json"。留空使用内置默认 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NXCharts")
	FString SpecPath;

	/** 解析后的配置。由 SpecPath 加载，也可用 SetChartSpec 运行时整体替换 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "NXCharts")
	FChartSpec Spec;

	/** 立即重新读取 SpecPath。失败时保留当前配置 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	bool ReloadSpec();

	/** 文件时间戳变了才重载，供画布轮询 */
	bool ReloadSpecIfChanged();

	FString GetSpecFullPath() const;

	/** 上次读到的文件修改时间 */
	FDateTime SpecFileTimeStamp;

	/** 是否自动重载：每 0.5 秒检查一次配置文件修改时间。打包后可以关掉 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NXCharts")
	bool bAutoReloadSpec = true;

	/** 每条系列最多保留的点数（<=0 表示不限制）。实时波形用它做滑动窗口 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NXCharts|Data")
	int32 SeriesCapacity = 0;

	// ---------------------------------------------------------------- 运行时配置

	/** 整体替换配置。幂等：同一份 Spec 应用多次结果一致 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetChartSpec(const FChartSpec& InSpec);

	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetAxisSpec(EChartAxis Axis, const FChartAxisSpec& AxisSpec);

	/** 按 SeriesKey 新增或更新一条系列。存在则改配置，不存在则追加 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void AddOrUpdateSeries(const FChartSeriesSpec& SeriesSpec);

	/** 删除一条系列（同时丢弃它的数据） */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void RemoveSeries(FName SeriesKey);

	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetSeriesVisible(FName SeriesKey, bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetSeriesColor(FName SeriesKey, FLinearColor Color);

	/**
	 * 一步到位：建配置 + 灌数据。
	 * 运行时增删系列最常用的入口——省掉在蓝图里手工构造嵌套结构体。
	 * Key 已存在时按 upsert 更新（保留原来的填充/发光等样式设置）。
	 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void AddSeries(FName SeriesKey, EChartSeriesKind Kind, FLinearColor Color,
	               const TArray<FVector2D>& Points, FString DisplayName);

	/** 整块替换样式（改曲线/填充/发光/线宽等，不动数据） */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetSeriesStyle(FName SeriesKey, const FChartSeriesStyle& Style);

	/** 只改连线方式（Linear 折线 / Smooth 平滑 / Step 阶梯），不动其它样式 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetSeriesCurve(FName SeriesKey, EChartCurveType Curve);

	/** 设置图例上显示的名字（不影响 SeriesKey 的数据绑定） */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void SetSeriesDisplayName(FName SeriesKey, const FString& DisplayName);

	/** 当前是否处于缩放/平移后的视图。外部复位按钮可以用它做显隐 */
	UFUNCTION(BlueprintPure, Category = "NXCharts|Interaction")
	bool IsViewModified() const;

	// ---------------------------------------------------------------- 系列查询

	UFUNCTION(BlueprintPure, Category = "NXCharts|Query")
	bool HasSeries(FName SeriesKey) const;

	/** 取一条系列的配置副本；不存在时返回默认结构（建议先用 HasSeries 判断） */
	UFUNCTION(BlueprintPure, Category = "NXCharts|Query")
	FChartSeriesSpec GetSeriesSpec(FName SeriesKey) const;

	// ---------------------------------------------------------------- 点级增删改查

	UFUNCTION(BlueprintPure, Category = "NXCharts|Query")
	int32 GetSeriesPointCount(FName SeriesKey) const;

	UFUNCTION(BlueprintPure, Category = "NXCharts|Query")
	TArray<FVector2D> GetSeriesPoints(FName SeriesKey) const;

	UFUNCTION(BlueprintPure, Category = "NXCharts|Query")
	bool GetSeriesPointAt(FName SeriesKey, int32 Index, FVector2D& OutPoint) const;

	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	bool SetSeriesPointAt(FName SeriesKey, int32 Index, FVector2D Point);

	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	bool InsertSeriesPoint(FName SeriesKey, int32 Index, FVector2D Point);

	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	bool RemoveSeriesPointAt(FName SeriesKey, int32 Index);

	/** 删除 X 落在 [MinX, MaxX] 闭区间内的点，返回删除数量。用于裁掉历史时间窗口 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	int32 RemoveSeriesPointsInXRange(FName SeriesKey, float MinX, float MaxX);

	// ---------------------------------------------------------------- 十字准星（外部驱动）

	/**
	 * 用「数据坐标」的 X 定位十字准星：吸附到最近的数据点，并显示各系列在该处的值。
	 * 激活期间鼠标悬停让位，由调用方负责关闭。
	 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts|Interaction")
	void SetCrosshairX(float X);

	UFUNCTION(BlueprintCallable, Category = "NXCharts|Interaction")
	void ClearCrosshair();

	UFUNCTION(BlueprintPure, Category = "NXCharts|Interaction")
	bool IsCrosshairActive() const;

	/** 供画布读取，也方便 C++ 侧查询 */
	double GetCrosshairX() const { return CrosshairX; }

	/** 十字准星吸附到数据点时广播（每条系列各一次），用于多图联动 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCrosshairSnapped, FName, SeriesKey, int32, PointIndex, FVector2D, DataValue);

	UPROPERTY(BlueprintAssignable, Category = "NXCharts|Events")
	FOnCrosshairSnapped OnCrosshairSnapped;

	/** 供 Slate 画布内部调用 */
	void BroadcastCrosshairSnapped(FName SeriesKey, int32 PointIndex, FVector2D DataValue);

	/** 强制重建并重绘（一般不需要手动调用） */
	UFUNCTION(BlueprintCallable, Category = "NXCharts")
	void RefreshChart();

	// ---------------------------------------------------------------- 运行时数据

	/** 整体替换某条系列的数据点 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	void SetSeriesPoints(FName SeriesKey, const TArray<FVector2D>& Points);

	/** 追加一个数据点（实时场景；C++ 侧有批量接口） */
	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	void AddSeriesPoint(FName SeriesKey, FVector2D Point);

	UFUNCTION(BlueprintCallable, Category = "NXCharts|Data")
	void ClearSeries(FName SeriesKey);

	// ---------------------------------------------------------------- 查询

	UFUNCTION(BlueprintPure, Category = "NXCharts")
	TArray<FName> GetSeriesKeys() const;

	// ---------------------------------------------------------------- 事件

	/** 悬停到某个数据点时触发。只在「最近点」变化时广播，鼠标在两点之间移动不会刷事件 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChartHovered, FName, SeriesKey, int32, PointIndex, FVector2D, DataValue);

	UPROPERTY(BlueprintAssignable, Category = "NXCharts|Events")
	FOnChartHovered OnChartHovered;

	/** 供 Slate 画布内部调用 */
	void BroadcastHover(FName SeriesKey, int32 PointIndex, FVector2D DataValue);

	// ---------------------------------------------------------------- 交互：运行时域覆盖

	/**
	 * 直接指定两根轴的范围（缩放/平移用）。
	 * 只想要一根轴时请用 C++ 的 SetDomainOverrideRaw。
	 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts|Interaction")
	void SetDomainOverride(FVector2D XRange, FVector2D YRange);

	/** 清除覆盖，回到 Spec 里的 Auto / Manual 行为 */
	UFUNCTION(BlueprintCallable, Category = "NXCharts|Interaction")
	void ClearDomainOverride();

	UFUNCTION(BlueprintPure, Category = "NXCharts|Interaction")
	bool HasDomainOverride() const;

	/** 供 Slate 画布调用：可以只覆盖单根轴 */
	void SetDomainOverrideRaw(const FChartDomainOverride& InOverride);
	const FChartDomainOverride& GetDomainOverride() const { return DomainOverride; }

	/** 绘制入口专用：取本帧快照，必要时重建。不要在游戏逻辑里调用 */
	const FChartFrame& GetFrame(const FVector2D& WidgetSize);

	/** C++ 批量追加入口，避免蓝图数组参数的整体拷贝 */
	void AddSeriesPoints(FName SeriesKey, TArrayView<const FVector2d> Points);

protected:
	// UWidget 重写
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual const FText GetPaletteCategory() override;
#endif

	void MarkSpecDirty();
	void InvalidateChart();

	// Slate 画布实例
	TSharedPtr<SChartCanvas> ChartCanvas;

	/** 数据通道：与配置分离，不进 Spec、不走反射 */
	FChartSeriesDataPool DataPool;

	/** 本帧快照缓存 */
	FChartFrame CachedFrame;

	/** 运行时域覆盖（缩放/平移） */
	FChartDomainOverride DomainOverride;

	/** 外部驱动的十字准星状态（数据坐标） */
	bool bCrosshairActive = false;
	double CrosshairX = 0.0;

	int32 SpecRevision = 0;
	int32 BuiltSpecRevision = -1;
	int64 BuiltDataRevisionSum = -1;
	FVector2D BuiltSize = FVector2D::ZeroVector;
};