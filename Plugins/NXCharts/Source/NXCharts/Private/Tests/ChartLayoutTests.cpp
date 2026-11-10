// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Frame/ChartCurve.h"
#include "Frame/ChartDecimator.h"
#include "Frame/ChartFrameBuilder.h"
#include "Layout/ChartSpace.h"
#include "Layout/ChartTicks.h"

// 这层是纯 C++（无 UObject、无 Slate 依赖），是整套插件里最适合单测的地方。

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsDomainTest, "NXCharts.Layout.Domain",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsDomainTest::RunTest(const FString&)
{
	FChartDomain Domain;
	TestFalse(TEXT("空域无效"), Domain.IsValid());

	Domain.Include(5.0);
	TestFalse(TEXT("跨度为 0 时仍无效"), Domain.IsValid());

	Domain.Normalize();
	TestTrue(TEXT("Normalize 后有效"), Domain.IsValid());
	TestTrue(TEXT("Normalize 后包含原值"), Domain.Contains(5.0));
	TestTrue(TEXT("Normalize 后向两侧撑开"), Domain.Min < 5.0 && Domain.Max > 5.0);

	// NaN 不应污染域
	Domain.Include(static_cast<double>(NAN));
	TestTrue(TEXT("NaN 被忽略"), Domain.IsValid());

	FChartDomain Padded = FChartDomain::FromRange(0.0, 100.0);
	Padded.Pad(0.1);
	TestEqual(TEXT("Pad 后 Min"), Padded.Min, -10.0, 1e-9);
	TestEqual(TEXT("Pad 后 Max"), Padded.Max, 110.0, 1e-9);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsTicksTest, "NXCharts.Layout.Ticks",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsTicksTest::RunTest(const FString&)
{
	// 1-2-5 步长
	TestEqual(TEXT("NiceStep(100,4)"), ChartTicks::NiceStep(100.0, 4), 50.0, 1e-9);
	TestEqual(TEXT("NiceStep(100,10)"), ChartTicks::NiceStep(100.0, 10), 10.0, 1e-9);
	TestEqual(TEXT("NiceStep(0.95,5)"), ChartTicks::NiceStep(0.95, 5), 0.2, 1e-9);
	TestEqual(TEXT("NiceStep(3,5)"), ChartTicks::NiceStep(3.0, 5), 1.0, 1e-9);
	TestEqual(TEXT("NiceStep(0,5) 退化为 0"), ChartTicks::NiceStep(0.0, 5), 0.0, 1e-9);

	// 小数位推导
	TestEqual(TEXT("step=100 -> 0 位小数"), ChartTicks::DecimalsForStep(100.0), 0);
	TestEqual(TEXT("step=0.1 -> 1 位小数"), ChartTicks::DecimalsForStep(0.1), 1);
	TestEqual(TEXT("step=0.05 -> 2 位小数"), ChartTicks::DecimalsForStep(0.05), 2);

	// 格式化
	TestTrue(TEXT("整千不丢精度"), ChartTicks::FormatValue(1234.0, 100.0, EChartTickFormat::Auto) == TEXT("1234"));
	TestTrue(TEXT("浮点噪声被抹平"), ChartTicks::FormatValue(0.30000000004, 0.1, EChartTickFormat::Auto) == TEXT("0.3"));
	TestTrue(TEXT("-0 归一为 0"), ChartTicks::FormatValue(-1e-12, 1.0, EChartTickFormat::Auto) == TEXT("0"));

	// 刻度生成：0..100，期望 6 个 -> 步长 20，正好 6 个主刻度
	const FChartDomain Domain = FChartDomain::FromRange(0.0, 100.0);
	const FChartTickSet TickSet = ChartTicks::MakeLinearTicks(Domain, 6, EChartTickFormat::Auto, true);

	TestEqual(TEXT("步长"), TickSet.Step, 20.0, 1e-9);
	TestEqual(TEXT("主刻度数量"), TickSet.NumMajor(), 6);
	TestTrue(TEXT("有次刻度"), TickSet.NumMinor() > 0);
	TestTrue(TEXT("首刻度为 0"), TickSet.Ticks.Num() > 0 && FMath::IsNearlyEqual(TickSet.Ticks[0].Value, 0.0, 1e-9));

	// 域不落在刻度上时，只保留域内的刻度
	const FChartDomain OddDomain = FChartDomain::FromRange(0.0, 97.0);
	const FChartTickSet OddSet = ChartTicks::MakeLinearTicks(OddDomain, 6, EChartTickFormat::Auto, false);
	TestEqual(TEXT("97 的步长"), OddSet.Step, 20.0, 1e-9);
	TestEqual(TEXT("97 内主刻度数量"), OddSet.NumMajor(), 5);

	// 退化域不应崩溃
	const FChartTickSet EmptySet = ChartTicks::MakeLinearTicks(FChartDomain(), 6, EChartTickFormat::Auto, true);
	TestTrue(TEXT("无效域返回空刻度"), EmptySet.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsSpaceTest, "NXCharts.Layout.Space",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsSpaceTest::RunTest(const FString&)
{
	FChartCartesianSpace Space;
	Space.Setup(FChartDomain::FromRange(0.0, 100.0), FChartDomain::FromRange(0.0, 1000.0),
	            FBox2D(FVector2D(100.0, 50.0), FVector2D(500.0, 350.0)));

	TestTrue(TEXT("空间有效"), Space.IsValid());

	// 中点映射到绘图区中心
	const FVector2D Local = Space.DataToLocal(FVector2D(50.0, 500.0));
	TestEqual(TEXT("中点 X"), Local.X, 300.0, 1e-6);
	TestEqual(TEXT("中点 Y"), Local.Y, 200.0, 1e-6);

	// 逆映射必须回到原值（这是 tooltip / 缩放锚点的前提）
	const FVector2D Back = Space.LocalToData(Local);
	TestEqual(TEXT("逆映射 X"), Back.X, 50.0, 1e-6);
	TestEqual(TEXT("逆映射 Y"), Back.Y, 500.0, 1e-6);

	// Y 轴翻转：数据 0 在底部，数据 Max 在顶部
	TestEqual(TEXT("Y=0 落在底部"), Space.DataYToLocal(0.0), 350.0, 1e-6);
	TestEqual(TEXT("Y=Max 落在顶部"), Space.DataYToLocal(1000.0), 50.0, 1e-6);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsFramePipelineTest, "NXCharts.Frame.Pipeline",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsFramePipelineTest::RunTest(const FString&)
{
	// 注入假测量：不依赖 Slate，宽度按字符数估算
	FChartMeasureFunc Measure = [](const FString& Text, const FSlateFontInfo&)
	{
		return FVector2D(Text.Len() * 6.0, 12.0);
	};

	FChartSpec Spec;
	Spec.Title.Text = FText::FromString(TEXT("Demo"));
	Spec.Legend.bShow = false;

	FChartSeriesSpec SeriesA;
	SeriesA.SeriesKey = TEXT("A");
	Spec.Series.Add(SeriesA);

	FChartSeriesSpec SeriesB;
	SeriesB.SeriesKey = TEXT("B");
	Spec.Series.Add(SeriesB);

	FChartSeriesDataPool Data;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(10.0, 10.0));
		Points.Add(FVector2d(20.0, 5.0));
		Data.FindOrAdd(TEXT("A")).SetPoints(Points);
	}
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 5.0));
		Points.Add(FVector2d(10.0, 15.0));
		Points.Add(FVector2d(20.0, 25.0));
		Data.FindOrAdd(TEXT("B")).SetPoints(Points);
	}

	const FVector2D WidgetSize(600.0, 400.0);

	// ---- 初始：两条系列
	FChartFrame Frame;
	TestTrue(TEXT("整帧构建成功"), FChartFrameBuilder::Build(Spec, Data, FChartDomainOverride(), WidgetSize, Frame, Measure));
	TestTrue(TEXT("帧有效"), Frame.bValid);
	TestEqual(TEXT("两条系列"), Frame.Series.Num(), 2);
	TestTrue(TEXT("绘图区有宽度"), Frame.Layout.PlotRect.Max.X > Frame.Layout.PlotRect.Min.X);
	TestEqual(TEXT("A 的顶点数"), Frame.Series[0].LocalPoints.Num(), 3);

	const FVector2d MidLocal = Frame.Series[0].LocalPoints[1];
	TestTrue(TEXT("顶点落在绘图区内"),
		MidLocal.X >= Frame.Layout.PlotRect.Min.X && MidLocal.X <= Frame.Layout.PlotRect.Max.X
		&& MidLocal.Y >= Frame.Layout.PlotRect.Min.Y && MidLocal.Y <= Frame.Layout.PlotRect.Max.Y);

	// ---- 运行时新增第三条系列（这就是「2 -> 3 条线」的核心路径）
	FChartSeriesSpec SeriesC;
	SeriesC.SeriesKey = TEXT("C");
	Spec.Series.Add(SeriesC);
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 1.0));
		Points.Add(FVector2d(20.0, 2.0));
		Data.FindOrAdd(TEXT("C")).SetPoints(Points);
	}

	TestTrue(TEXT("协调后帧仍有效"),
		FChartFrameBuilder::ReconcileData(Frame, Spec, Data, FChartDomainOverride(), WidgetSize, Measure));
	TestEqual(TEXT("协调后三条系列"), Frame.Series.Num(), 3);

	// ---- 只改 A 的数据且不改变域 -> 走增量路径
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(10.0, 8.0));
		Points.Add(FVector2d(20.0, 5.0));
		Data.FindOrAdd(TEXT("A")).SetPoints(Points);
	}

	TestTrue(TEXT("增量协调成功"),
		FChartFrameBuilder::ReconcileData(Frame, Spec, Data, FChartDomainOverride(), WidgetSize, Measure));
	TestEqual(TEXT("仍是三条系列"), Frame.Series.Num(), 3);
	TestEqual(TEXT("A 的几何版本已更新"), Frame.Series[0].DataRevision, Data.Find(TEXT("A"))->Revision);
	TestEqual(TEXT("B 的几何版本未变"), Frame.Series[1].DataRevision, Data.Find(TEXT("B"))->Revision);

	// ---- 数据把域撑开时必须整帧重建（刻度与布局都会变）
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(100.0, 500.0));
		Data.FindOrAdd(TEXT("C")).SetPoints(Points);
	}

	TestTrue(TEXT("域变化后仍有效"),
		FChartFrameBuilder::ReconcileData(Frame, Spec, Data, FChartDomainOverride(), WidgetSize, Measure));
	TestTrue(TEXT("域被新数据撑开"), Frame.Space.GetYDomain().Max >= 500.0);
	TestEqual(TEXT("系列数不变"), Frame.Series.Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsDecimateTest, "NXCharts.Frame.Decimate",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsDecimateTest::RunTest(const FString&)
{
	TArray<FVector2d> Points;
	Points.Reserve(10000);
	for (int32 Index = 0; Index < 10000; ++Index)
	{
		Points.Add(FVector2d(Index, FMath::Sin(Index * 0.01) * 100.0));
	}

	TArray<int32> Indices;

	// 数据量低于阈值：不抽稀
	ChartDecimate::MinMaxByColumn(Points, 0.0, 9999.0, 100.0, 20000, Indices);
	TestEqual(TEXT("未达阈值时全保留"), Indices.Num(), Points.Num());

	// 绘图区宽 200 像素：顶点数应被压到 400 以内
	ChartDecimate::MinMaxByColumn(Points, 0.0, 9999.0, 200.0, 400, Indices);
	TestTrue(TEXT("发生了抽稀"), Indices.Num() < Points.Num());
	TestTrue(TEXT("顶点数受绘图区宽度约束"), Indices.Num() <= 440);

	bool bAscending = true;
	for (int32 Index = 1; Index < Indices.Num(); ++Index)
	{
		if (Indices[Index] <= Indices[Index - 1])
		{
			bAscending = false;
			break;
		}
	}
	TestTrue(TEXT("索引严格升序"), bAscending);

	TestTrue(TEXT("覆盖数据头部"), Indices.Num() > 0 && Indices[0] < 100);
	TestTrue(TEXT("覆盖数据尾部"), Indices.Num() > 0 && Indices.Last() > 9800);

	// 退化域不崩溃，退化为全量
	ChartDecimate::MinMaxByColumn(Points, 5.0, 5.0, 200.0, 400, Indices);
	TestEqual(TEXT("退化域退化为全量"), Indices.Num(), Points.Num());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsDomainOverrideTest, "NXCharts.Frame.DomainOverride",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsDomainOverrideTest::RunTest(const FString&)
{
	FChartSeriesDataPool Data;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(100.0, 100.0));
		Data.FindOrAdd(TEXT("A")).SetPoints(Points);
	}

	FChartSpec Spec;
	FChartSeriesSpec SeriesSpec;
	SeriesSpec.SeriesKey = TEXT("A");
	Spec.Series.Add(SeriesSpec);

	FChartDomainOverride Override;
	Override.bOverrideX = true;
	Override.XRange = FVector2D(10.0, 20.0);

	const FChartDomain X = FChartFrameBuilder::ResolveDomain(EChartAxis::X, Spec.AxisX, Spec, Data, Override);
	TestEqual(TEXT("X 使用覆盖域下界"), X.Min, 10.0, 1e-9);
	TestEqual(TEXT("X 使用覆盖域上界"), X.Max, 20.0, 1e-9);

	const FChartDomain Y = FChartFrameBuilder::ResolveDomain(EChartAxis::Y, Spec.AxisY, Spec, Data, Override);
	TestTrue(TEXT("Y 仍走自动域"), Y.Max >= 100.0);

	// Manual 优先于 Auto
	Spec.AxisX.DomainMode = EChartDomainMode::Manual;
	Spec.AxisX.ManualDomain = FVector2D(0.0, 5.0);

	const FChartDomain ManualX = FChartFrameBuilder::ResolveDomain(
		EChartAxis::X, Spec.AxisX, Spec, Data, FChartDomainOverride());
	TestEqual(TEXT("Manual 范围生效"), ManualX.Max, 5.0, 1e-9);

	// 但 Override 优先于 Manual
	const FChartDomain OverrideWins = FChartFrameBuilder::ResolveDomain(
		EChartAxis::X, Spec.AxisX, Spec, Data, Override);
	TestEqual(TEXT("Override 优先于 Manual"), OverrideWins.Max, 20.0, 1e-9);

	// 清除后回到 Manual
	FChartDomainOverride Cleared;
	Cleared.Clear();
	const FChartDomain AfterClear = FChartFrameBuilder::ResolveDomain(
		EChartAxis::X, Spec.AxisX, Spec, Data, Cleared);
	TestEqual(TEXT("Clear 后回到 Manual"), AfterClear.Max, 5.0, 1e-9);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsLabelFormatTest, "NXCharts.Layout.LabelFormat",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsLabelFormatTest::RunTest(const FString&)
{
	FChartMeasureFunc Measure = [](const FString& Text, const FSlateFontInfo&)
	{
		return FVector2D(Text.Len() * 6.0, 12.0);
	};

	FChartSeriesSpec SeriesSpec;
	SeriesSpec.SeriesKey = TEXT("reset");
	SeriesSpec.DisplayName = FText::FromString(TEXT("复位"));

	TestTrue(TEXT("图例优先用 DisplayName"), SeriesSpec.GetLegendLabel() == TEXT("复位"));

	FChartSeriesSpec NoDisplay;
	NoDisplay.SeriesKey = TEXT("rpm");
	TestTrue(TEXT("没填 DisplayName 时回退到 SeriesKey"), NoDisplay.GetLegendLabel() == TEXT("rpm"));

	FChartSpec Spec;
	Spec.Legend.bShow = false;
	Spec.AxisX.TickSuffix = TEXT("s");
	Spec.Series.Add(SeriesSpec);

	FChartSeriesDataPool Data;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(3.0, 100.0));
		Data.FindOrAdd(TEXT("reset")).SetPoints(Points);
	}

	FChartFrame Frame;
	TestTrue(TEXT("帧构建成功"),
		FChartFrameBuilder::Build(Spec, Data, FChartDomainOverride(), FVector2D(400.0, 300.0), Frame, Measure));
	TestTrue(TEXT("帧有效"), Frame.bValid);

	int32 MajorCount = 0;
	bool bAllSuffixed = true;
	for (const FChartTick& Tick : Frame.Layout.XAxis.TickSet.Ticks)
	{
		if (!Tick.bMajor)
		{
			continue;
		}
		++MajorCount;
		if (!Tick.Label.EndsWith(TEXT("s")))
		{
			bAllSuffixed = false;
		}
	}

	TestTrue(TEXT("有主刻度"), MajorCount > 0);
	TestTrue(TEXT("X 主刻度都带 s 后缀"), bAllSuffixed);

	if (Frame.Layout.YAxis.TickSet.Ticks.Num() > 0)
	{
		TestFalse(TEXT("Y 轴没有配置后缀，不应带 s"),
			Frame.Layout.YAxis.TickSet.Ticks[0].Label.EndsWith(TEXT("s")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsCurveTest, "NXCharts.Frame.Curve",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsCurveTest::RunTest(const FString&)
{
	TArray<FVector2d> Line;
	Line.Add(FVector2d(0.0, 0.0));
	Line.Add(FVector2d(10.0, 100.0));
	Line.Add(FVector2d(20.0, 0.0));

	TArray<FVector2d> Smooth;
	ChartCurve::SmoothMonotone(Line, 20000, Smooth);

	TestTrue(TEXT("细分后点数增加"), Smooth.Num() > Line.Num());
	TestTrue(TEXT("首点保留"), Smooth.Num() > 0 && Smooth[0].Equals(Line[0]));
	TestTrue(TEXT("末点保留"), Smooth.Num() > 0 && Smooth.Last().Equals(Line.Last()));

	// 单调三次插值的关键性质：不在相邻点之间过冲
	bool bInRange = true;
	for (const FVector2d& Point : Smooth)
	{
		if (Point.Y < -1e-6 || Point.Y > 100.0 + 1e-6)
		{
			bInRange = false;
			break;
		}
	}
	TestTrue(TEXT("没有过冲"), bInRange);

	// X 必须保持单调，否则曲线会回头
	bool bMonotonicX = true;
	for (int32 Index = 1; Index < Smooth.Num(); ++Index)
	{
		if (Smooth[Index].X < Smooth[Index - 1].X - 1e-9)
		{
			bMonotonicX = false;
			break;
		}
	}
	TestTrue(TEXT("X 保持单调"), bMonotonicX);

	// 阶梯
	TArray<FVector2d> StepPoints;
	ChartCurve::Step(Line, StepPoints);
	TestEqual(TEXT("阶梯点数 = 2N-1"), StepPoints.Num(), Line.Num() * 2 - 1);
	TestTrue(TEXT("阶梯先水平走"), StepPoints.Num() > 1 && FMath::IsNearlyEqual(StepPoints[1].Y, Line[0].Y));

	// 少于 3 点退化为原样
	TArray<FVector2d> TwoPoints;
	TwoPoints.Add(FVector2d(0.0, 0.0));
	TwoPoints.Add(FVector2d(1.0, 1.0));
	TArray<FVector2d> TwoOut;
	ChartCurve::SmoothMonotone(TwoPoints, 20000, TwoOut);
	TestEqual(TEXT("两点不做细分"), TwoOut.Num(), 2);

	// 乱序数据必须退化为折线，不能画出回头线
	TArray<FVector2d> Unordered;
	Unordered.Add(FVector2d(10.0, 0.0));
	Unordered.Add(FVector2d(0.0, 5.0));
	Unordered.Add(FVector2d(20.0, 1.0));
	TArray<FVector2d> UnorderedOut;
	ChartCurve::SmoothMonotone(Unordered, 20000, UnorderedOut);
	TestEqual(TEXT("乱序退化为折线"), UnorderedOut.Num(), Unordered.Num());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsStyleFlagsTest, "NXCharts.Frame.StyleFlags",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsStyleFlagsTest::RunTest(const FString&)
{
	FChartMeasureFunc Measure = [](const FString& Text, const FSlateFontInfo&)
	{
		return FVector2D(Text.Len() * 6.0, 12.0);
	};

	FChartSpec Spec;
	Spec.Legend.bShow = false;
	Spec.Background.bGradient = true;

	FChartSeriesSpec SeriesSpec;
	SeriesSpec.SeriesKey = TEXT("A");
	SeriesSpec.Style.bFillArea = true;
	SeriesSpec.Style.AreaFillOpacity = 0.3f;
	SeriesSpec.Style.bGlow = true;
	SeriesSpec.Style.GlowThickness = 12.0f;
	SeriesSpec.Style.Curve = EChartCurveType::Smooth;
	Spec.Series.Add(SeriesSpec);

	FChartSeriesDataPool Data;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(10.0, 50.0));
		Points.Add(FVector2d(20.0, 20.0));
		Data.FindOrAdd(TEXT("A")).SetPoints(Points);
	}

	FChartFrame Frame;
	TestTrue(TEXT("帧构建成功"),
		FChartFrameBuilder::Build(Spec, Data, FChartDomainOverride(), FVector2D(600.0, 400.0), Frame, Measure));
	TestTrue(TEXT("帧有效"), Frame.bValid);
	TestEqual(TEXT("一条系列"), Frame.Series.Num(), 1);

	TestTrue(TEXT("填充标志透传到几何"), Frame.Series[0].bFillArea);
	TestEqual(TEXT("填充透明度透传"), Frame.Series[0].AreaFillOpacity, 0.3f, 1e-4f);
	TestTrue(TEXT("发光标志透传到几何"), Frame.Series[0].bGlow);
	TestEqual(TEXT("发光宽度透传"), Frame.Series[0].GlowThickness, 12.0f, 1e-4f);
	TestTrue(TEXT("平滑曲线使顶点数增加"), Frame.Series[0].LocalPoints.Num() > 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsLegendLayoutTest, "NXCharts.Layout.Legend",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsLegendLayoutTest::RunTest(const FString&)
{
	FChartMeasureFunc Measure = [](const FString& Text, const FSlateFontInfo&)
	{
		return FVector2D(Text.Len() * 6.0, 12.0);
	};

	FChartSpec Spec;
	Spec.Legend.bShow = true;
	Spec.Legend.Position = EChartLegendPosition::Bottom;

	FChartSeriesSpec SeriesA;
	SeriesA.SeriesKey = TEXT("A");
	SeriesA.DisplayName = FText::FromString(TEXT("复位"));
	Spec.Series.Add(SeriesA);

	FChartSeriesSpec SeriesB;
	SeriesB.SeriesKey = TEXT("B");
	SeriesB.DisplayName = FText::FromString(TEXT("单位m/s"));
	SeriesB.bVisible = false;
	Spec.Series.Add(SeriesB);

	FChartSeriesDataPool Data;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(1.0, 10.0));
		Data.FindOrAdd(TEXT("A")).SetPoints(Points);
	}

	FChartFrame Frame;
	TestTrue(TEXT("帧构建成功"),
		FChartFrameBuilder::Build(Spec, Data, FChartDomainOverride(), FVector2D(600.0, 400.0), Frame, Measure));
	TestTrue(TEXT("帧有效"), Frame.bValid);
	TestTrue(TEXT("图例存在"), Frame.Layout.bHasLegend);

	// 关键：隐藏的系列仍要留在图例里，否则用户没有入口把它点回来
	TestEqual(TEXT("隐藏的系列也在图例里"), Frame.Layout.LegendItems.Num(), 2);

	if (Frame.Layout.LegendItems.Num() == 2)
	{
		TestTrue(TEXT("条目顺序与 Spec 一致"), Frame.Layout.LegendItems[0].SeriesKey == FName(TEXT("A")));

		const FBox2D& First = Frame.Layout.LegendItems[0].Rect;
		const FBox2D& Second = Frame.Layout.LegendItems[1].Rect;

		TestTrue(TEXT("条目有宽度"), First.Max.X > First.Min.X);
		TestTrue(TEXT("条目有高度"), First.Max.Y > First.Min.Y);
		TestTrue(TEXT("横向排列不重叠"), Second.Min.X >= First.Max.X - 1e-6);
		TestTrue(TEXT("色块落在条目内"),
			Frame.Layout.LegendItems[0].MarkerRect.Min.X >= First.Min.X - 1e-6
			&& Frame.Layout.LegendItems[0].MarkerRect.Max.X <= First.Max.X + 1e-6);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsHardLimitsTest, "NXCharts.Frame.HardLimits",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsHardLimitsTest::RunTest(const FString&)
{
	FChartSeriesDataPool Data;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(100.0, 1500.0));
		Data.FindOrAdd(TEXT("A")).SetPoints(Points);
	}

	FChartSpec Spec;
	FChartSeriesSpec SeriesSpec;
	SeriesSpec.SeriesKey = TEXT("A");
	Spec.Series.Add(SeriesSpec);

	// 数据 0~1500，自动域带 5% padding 会变成 -75~1575：这就是负刻度的来源
	const FChartDomain Unclamped = FChartFrameBuilder::ResolveDomain(
		EChartAxis::Y, Spec.AxisY, Spec, Data, FChartDomainOverride());
	TestTrue(TEXT("自动域默认会向下溢出到负数"), Unclamped.Min < 0.0);

	// 打开取值范围限制后不允许小于 0
	Spec.AxisY.bUseHardLimits = true;
	Spec.AxisY.HardLimits = FVector2D(0.0, 1500.0);

	const FChartDomain Clamped = FChartFrameBuilder::ResolveDomain(
		EChartAxis::Y, Spec.AxisY, Spec, Data, FChartDomainOverride());
	TestEqual(TEXT("自动域下界被夹到 0"), Clamped.Min, 0.0, 1e-6);
	TestEqual(TEXT("自动域上界被夹到 1500"), Clamped.Max, 1500.0, 1e-6);

	// 缩放产生的运行时覆盖同样要被夹住（防止越缩越远）
	FChartDomainOverride Override;
	Override.bOverrideY = true;
	Override.YRange = FVector2D(-3000.0, 4500.0);

	const FChartDomain OverrideClamped = FChartFrameBuilder::ResolveDomain(
		EChartAxis::Y, Spec.AxisY, Spec, Data, Override);
	TestEqual(TEXT("缩放覆盖同样被夹住下界"), OverrideClamped.Min, 0.0, 1e-6);
	TestEqual(TEXT("缩放覆盖同样被夹住上界"), OverrideClamped.Max, 1500.0, 1e-6);

	// 只限一根轴时，另一根轴不受影响
	const FChartDomain XUnaffected = FChartFrameBuilder::ResolveDomain(
		EChartAxis::X, Spec.AxisX, Spec, Data, FChartDomainOverride());
	TestTrue(TEXT("X 轴未配置限制时保持自动域"), XUnaffected.Max >= 100.0);

	// 数据完全落在限制区之外：退回展示整个限制区间，而不是空白
	Spec.AxisY.HardLimits = FVector2D(5000.0, 6000.0);
	const FChartDomain Outside = FChartFrameBuilder::ResolveDomain(
		EChartAxis::Y, Spec.AxisY, Spec, Data, FChartDomainOverride());
	TestEqual(TEXT("数据全在区外时退回限制区间下界"), Outside.Min, 5000.0, 1e-6);
	TestEqual(TEXT("数据全在区外时退回限制区间上界"), Outside.Max, 6000.0, 1e-6);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsPointCrudTest, "NXCharts.Data.PointCrud",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsPointCrudTest::RunTest(const FString&)
{
	FChartSeriesBuffer Buffer;
	{
		TArray<FVector2d> Points;
		Points.Add(FVector2d(0.0, 0.0));
		Points.Add(FVector2d(1.0, 10.0));
		Points.Add(FVector2d(2.0, 20.0));
		Buffer.SetPoints(Points);
	}

	const int32 RevisionAfterSet = Buffer.Revision;

	// ---- 查
	TestEqual(TEXT("点数"), Buffer.Num(), 3);
	TestTrue(TEXT("按下标可取点"), Buffer.GetPointAt(1) != nullptr);
	TestTrue(TEXT("取到的值正确"), Buffer.GetPointAt(1) && FMath::IsNearlyEqual(Buffer.GetPointAt(1)->Y, 10.0));
	TestTrue(TEXT("越界返回空"), Buffer.GetPointAt(99) == nullptr);

	// ---- 改
	TestTrue(TEXT("改点成功"), Buffer.SetPointAt(1, FVector2d(1.0, 15.0)));
	TestTrue(TEXT("改点后值更新"), FMath::IsNearlyEqual(Buffer.GetPointAt(1)->Y, 15.0));
	TestTrue(TEXT("改点使版本递增"), Buffer.Revision > RevisionAfterSet);
	TestFalse(TEXT("越界改点失败"), Buffer.SetPointAt(99, FVector2d::ZeroVector));

	// ---- 增
	TestTrue(TEXT("插入成功"), Buffer.InsertPoint(1, FVector2d(0.5, 5.0)));
	TestEqual(TEXT("插入后点数"), Buffer.Num(), 4);
	TestTrue(TEXT("插入位置正确"), Buffer.GetPointAt(1) && FMath::IsNearlyEqual(Buffer.GetPointAt(1)->X, 0.5));
	TestFalse(TEXT("越界插入失败"), Buffer.InsertPoint(99, FVector2d::ZeroVector));

	// ---- 删
	TestTrue(TEXT("按下标删除"), Buffer.RemovePointAt(1));
	TestEqual(TEXT("删除后点数"), Buffer.Num(), 3);
	TestFalse(TEXT("越界删除失败"), Buffer.RemovePointAt(99));

	// ---- 按 X 区间批量删（裁历史窗口）
	TestEqual(TEXT("区间删除数量"), Buffer.RemovePointsInXRange(0.0, 1.0), 2);
	TestEqual(TEXT("区间删除后剩余"), Buffer.Num(), 1);
	TestEqual(TEXT("空区间不删"), Buffer.RemovePointsInXRange(5.0, 6.0), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsNearestPointTest, "NXCharts.Frame.NearestPoint",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsNearestPointTest::RunTest(const FString&)
{
	TArray<FVector2d> LocalPoints;
	LocalPoints.Add(FVector2d(0.0, 0.0));
	LocalPoints.Add(FVector2d(10.0, 0.0));
	LocalPoints.Add(FVector2d(20.0, 0.0));

	TestEqual(TEXT("命中中间点"), ChartQuery::FindNearestPointIndex(LocalPoints, 9.0), 1);
	TestEqual(TEXT("偏左侧仍命中中间"), ChartQuery::FindNearestPointIndex(LocalPoints, 6.0), 1);
	TestEqual(TEXT("命中首点"), ChartQuery::FindNearestPointIndex(LocalPoints, -50.0), 0);
	TestEqual(TEXT("命中末点"), ChartQuery::FindNearestPointIndex(LocalPoints, 100.0), 2);

	const TArray<FVector2d> Empty;
	TestEqual(TEXT("空数组返回 INDEX_NONE"), ChartQuery::FindNearestPointIndex(Empty, 5.0), static_cast<int32>(INDEX_NONE));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNXChartsDomainClampTest, "NXCharts.Layout.Clamp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNXChartsDomainClampTest::RunTest(const FString&)
{
	const FChartDomain AtLimits = FChartDomain::FromRange(0.0, 1500.0);
	const FChartDomain ZoomedIn = FChartDomain::FromRange(300.0, 1200.0);

	// 1) 限制内放大：必须被接受（旧实现的 bug 就出在这里）
	FChartDomain Domain = ZoomedIn;
	TestTrue(TEXT("限制内放大应被接受"), ChartDomainUtil::ClampDomainToRange(Domain, 0.0, 1500.0, AtLimits));
	TestEqual(TEXT("放大后下界保持"), Domain.Min, 300.0, 1e-6);
	TestEqual(TEXT("放大后上界保持"), Domain.Max, 1200.0, 1e-6);

	// 2) 越界缩小：接受一次并夹到边界
	FChartDomain ZoomOut = FChartDomain::FromRange(-500.0, 2000.0);
	TestTrue(TEXT("越界缩小应被接受并夹紧"),
		ChartDomainUtil::ClampDomainToRange(ZoomOut, 0.0, 1500.0, ZoomedIn));
	TestEqual(TEXT("夹到限制下界"), ZoomOut.Min, 0.0, 1e-6);
	TestEqual(TEXT("夹到限制上界"), ZoomOut.Max, 1500.0, 1e-6);

	// 3) 已经顶死在边界：再缩应被拒绝
	FChartDomain Again = FChartDomain::FromRange(-100.0, 1700.0);
	TestFalse(TEXT("顶死边界时拒绝继续缩小"),
		ChartDomainUtil::ClampDomainToRange(Again, 0.0, 1500.0, AtLimits));

	// 4) 数据整体落在限制区之外：退化成整个限制区间
	FChartDomain Outside = FChartDomain::FromRange(5000.0, 6000.0);
	TestTrue(TEXT("完全区外时退化并接受"),
		ChartDomainUtil::ClampDomainToRange(Outside, 0.0, 1500.0, ZoomedIn));
	TestEqual(TEXT("退化后下界"), Outside.Min, 0.0, 1e-6);
	TestEqual(TEXT("退化后上界"), Outside.Max, 1500.0, 1e-6);

	// 5) 无效限制区间（Lower == Upper）不参与夹紧
	FChartDomain Free = FChartDomain::FromRange(-800.0, 900.0);
	TestTrue(TEXT("无效限制时不夹紧"),
		ChartDomainUtil::ClampDomainToRange(Free, 100.0, 100.0, AtLimits));
	TestEqual(TEXT("无效限制时域保持不变"), Free.Min, -800.0, 1e-6);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
