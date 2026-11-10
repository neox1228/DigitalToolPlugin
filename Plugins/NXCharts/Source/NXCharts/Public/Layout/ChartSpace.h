// Fill out your copyright notice in the Description page of Project Settings.

/**
 * @file    ChartSpace.h
 * @brief   数据域（domain）与笛卡尔坐标空间。
 *
 * 这是「数据坐标 <-> 控件局部坐标」的唯一事实源：
 *  - 绘制用 DataToLocal
 *  - tooltip / 框选 / 缩放锚点用 LocalToData
 * 两者必须来自同一个对象，否则会出现"图形在这里、交互在那里"的漂移。
 *
 * 纯 C++，不依赖 Slate，可直接单测。
 */

#pragma once

#include "CoreMinimal.h"

/** 一段有效的数据区间。轴范围、自动范围、缩放窗口都收敛到这里 */
struct NXCHARTS_API FChartDomain
{
	double Min = 0.0;
	double Max = 1.0;
	bool bValid = false;

	void Reset();

	/** 纳入一个数据点。非有限值（NaN/Inf）被忽略，不会污染域 */
	void Include(double Value);
	void Include(const FChartDomain& Other);

	bool IsValid() const;
	double Size() const;

	/** 修正退化域：Max < Min 时交换，跨度为零时按比例撑开（所有点相等也要能画） */
	void Normalize(double MinSpanRatio = 0.1);

	/** 上下各扩 Ratio * Size */
	void Pad(double Ratio);

	bool Contains(double Value) const;

	static FChartDomain FromRange(double InMin, double InMax);
};

/**
 * 运行时域覆盖：缩放/平移改的是它，不是数据本身。
 *
 * 优先级：Override > AxisSpec.ManualDomain > 自动推导。
 * 这样"用户拖过视图"和"配置里写死了范围"不会互相打架，Clear 一下即可回到自动域。
 */
struct NXCHARTS_API FChartDomainOverride
{
	bool bOverrideX = false;
	FVector2D XRange = FVector2D(0.0, 1.0);

	bool bOverrideY = false;
	FVector2D YRange = FVector2D(0.0, 1.0);

	bool HasAny() const { return bOverrideX || bOverrideY; }

	void Clear()
	{
		bOverrideX = false;
		bOverrideY = false;
	}
};

/** 域的通用操作（纯函数，便于单测） */
namespace ChartDomainUtil
{
	/**
	 * 把 Domain 夹进 [Lower, Upper]，并判断"这次变更是否值得应用"。
	 *
	 * 关键点：比较基准是 Current（当前正在显示的域），不是夹紧之前的 Domain。
	 * 早期版本拿夹紧前后比，导致"限制内放大"被误判成"已到边界"而丢弃。
	 *
	 * @return false 表示夹紧后与当前域没有区别（已顶死在边界），调用方应忽略本次缩放
	 */
	NXCHARTS_API bool ClampDomainToRange(FChartDomain& Domain, double Lower, double Upper, const FChartDomain& Current);
}

/**
 * 笛卡尔坐标空间。
 * 约定：局部坐标 Y 轴向下、数据 Y 轴向上，所以 Y 映射是翻转的。
 */
struct NXCHARTS_API FChartCartesianSpace
{
	void Setup(const FChartDomain& InXDomain, const FChartDomain& InYDomain, const FBox2D& InPlotRect);

	bool IsValid() const;

	double DataXToLocal(double X) const;
	double DataYToLocal(double Y) const;
	FVector2D DataToLocal(const FVector2D& Data) const;

	FVector2D LocalToData(const FVector2D& Local) const;

	const FChartDomain& GetXDomain() const { return XDomain; }
	const FChartDomain& GetYDomain() const { return YDomain; }
	const FBox2D& GetPlotRect() const { return PlotRect; }

private:
	FChartDomain XDomain;
	FChartDomain YDomain;
	FBox2D PlotRect = FBox2D(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0));
};