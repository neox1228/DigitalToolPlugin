# NXCharts 插件手册

> 版本：阶段 A/B 完成（含主题化、平滑曲线、渐变填充、悬停、缩放平移、图例联动）
> 引擎：**UE 5.8** · 模块：`NXCharts`（Runtime，无编辑器依赖）
> 本文涵盖：架构 · 技术栈 · 使用 · 扩展

---

## 1. 这是什么

一个 UMG 原生图表控件。**不依赖 WebView、不依赖第三方库**，用 Slate 直接画，在蓝图里配。

它要解决的问题不是"画一条线"，而是**让图表成为一种可以被系统驱动的运行时对象**：

- 图表长什么样，是一份**可序列化的值**（`FChartSpec`），不是资产里的一堆子对象
- 运行时可以增删系列、换样式、改数据，**不需要回 UMG、不需要重新打包**
- 数据与配置分离，高频数据更新不会拖着配置系统一起跑
- 绘制只读一份不可变快照，绘制期不做任何 O(数据量) 的计算

---

## 2. 技术栈

| 层 | 用了什么 | 说明 |
| --- | --- | --- |
| UI 容器 | `UWidget` + `SLeafWidget` | UMG 负责生命周期与蓝图入口，Slate 负责绘制与输入 |
| 绘制 | `FSlateDrawElement::MakeLines / MakeBox / MakeText` | 线、矩形、文字 |
| 渐变 | `FSlateDrawElement::MakeCustomVerts` | Slate 无渐变笔刷，用顶点色插值实现（顶点是**窗口空间**坐标） |
| 裁剪 | `FSlateClippingZone` + `PushClip/PopClip` | 曲线、网格、渐变都裁在绘图区内 |
| 反射 | `USTRUCT/UENUM/UPROPERTY/UFUNCTION` | Spec 与事件对蓝图可见 |
| 文字度量 | `FSlateFontMeasure` | 布局按真实文字宽度推绘图区，不拍常量内边距 |
| 测试 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST` | 10 组，纯逻辑可无渲染跑 |
| 引擎 | UE 5.8（要求 `BuildSettingsVersion.V7`） | 见 §10 |

**没有第三方依赖。** 唯一的平台假设是 Slate 存在（专用服务器上文字测量返回 0，不会崩）。

---

## 3. 架构

### 3.1 分层

```
      配置通道（低频）                          数据通道（高频）
   FChartSpec（USTRUCT，纯值）            FChartSeriesDataPool（TMap<FName, Buffer>）
        │  ApplySpec / AddSeries               │  SetSeriesPoints / AddSeriesPoint
        └──────────────┬───────────────────────┘
                       ▼
            FChartFrameBuilder（编译层）
              域 → 刻度 → 布局 → 几何
                       ▼
              FChartFrame（POD 快照，OnPaint 只读）
                       ▼
        SChartCanvas::OnPaint → FChartPaintContext
```

四条硬约束，也是判断"某个改动该放哪一层"的标准：

1. **资产里只存值**，不存对象图（Widget BP 上只有 `Spec` 一个结构体）
2. **轴 / 网格 / 图例 / 标题由帧生成**，不是用户装配的对象
3. **OnPaint 里没有 O(数据点) 的计算，也不 NewObject**
4. **坐标映射只有一个事实源**（`FChartCartesianSpace`），绘制与交互共用

### 3.2 帧的生命周期

`UNXChartWidget::GetFrame(WidgetSize)` 是唯一的重建闸门，比较三个版本：

| 变化项 | 触发 | 代价 |
| --- | --- | --- |
| `SpecRevision` | Spec 被改、尺寸变化 | **整帧重建**：域 → 刻度 → 布局 → 全部几何 |
| 数据 Revision 之和 | 只有数据变了 | **增量协调**：域没变就只重建变化的那几条系列几何，布局与文字测量全部跳过 |
| 都没有 | 纯绘制 | 直接返回缓存帧 |

数据一变就把自动域撑开的情况（新数据超出原范围），会退化成整帧重建——因为刻度、标签、绘图区都要跟着变，这是正确行为。

### 3.3 几何管线（顺序不能改）

```
原始点 → 抽稀（按像素列 min/max）→ 映射到局部坐标 → 曲线细分 → 绘制
```

- **抽稀必须在映射前**：先把 O(数据量) 压到 O(绘图区宽度)，否则 10 万点会先被全量映射一遍
- **曲线细分必须在抽稀后**：否则 1 万点 × 每段 8 采样 = 8 万顶点
- 平滑用 **Fritsch–Carlson 单调三次插值**：保证相邻两点之间不过冲，不会画出数据里不存在的尖峰

### 3.4 布局

布局不是"算个矩形"，而是由文字实测反推：

```
Padding → 标题带 → 图例带 → 刻度（nice numbers）→ 测最宽标签 → 推 PlotRect
```

所以 Y 轴标签从 `1` 变成 `1200000` 时，绘图区会自动让出更多左侧空间。

图例条目矩形也在这里产出（`FChartLayout::LegendItems`），**绘制与点击命中共用同一份**，避免两处各算一遍产生漂移。

### 3.5 绘制

`FChartPaintContext` 是唯一的绘制出口，对外只暴露「线 / 虚线 / 矩形 / 描边 / 圆 / 文字 / 渐变 / 裁剪」。

绘制顺序固定为：背景 → 网格 → 坐标轴与标签 → 曲线（填充单独一遍先画）→ 图例 → 悬停覆盖层。

> 面积填充必须单独一遍先画完，否则后画的填充会盖住前一条系列的线。

---

## 4. 目录结构

```
Plugins/NXCharts/
├─ NXCharts.uplugin
├─ Docs/
│  ├─ NXCharts_Manual.md          ← 本文
│  └─ Archive/                    ← 历史设计文档（评审、V2 方案、过程记录）
└─ Source/NXCharts/
   ├─ NXCharts.Build.cs
   ├─ Public/
   │  ├─ NXCharts.h                模块入口
   │  ├─ Core/
   │  │  ├─ NXChartsTypes.h        公共枚举
   │  │  ├─ ChartPaintContext.h    绘制收敛层
   │  │  └─ NXChartTheme.h         主题资产（预留）
   │  ├─ Spec/
   │  │  ├─ NXChartSpecTypes.h     系列/轴的配置
   │  │  └─ NXChartSpec.h          配置根对象
   │  ├─ Data/
   │  │  └─ ChartSeriesBuffer.h    高频数据通道
   │  ├─ Layout/
   │  │  ├─ ChartSpace.h           域与坐标空间
   │  │  ├─ ChartTicks.h           刻度生成
   │  │  └─ ChartLayout.h          布局
   │  ├─ Frame/
   │  │  └─ ChartFrame.h           帧快照
   │  └─ Widget/
   │     └─ NXChartWidget.h        UMG 入口（类 UNXChartWidget）
   └─ Private/
      ├─ Core/     ChartPaintContext.cpp · ChartTheme.cpp
      ├─ Data/     ChartSeriesBuffer.cpp
      ├─ Layout/   ChartSpace.cpp · ChartTicks.cpp · ChartLayout.cpp
      ├─ Frame/    ChartFrameBuilder.* · ChartDecimator.* · ChartCurve.*
      ├─ Tests/    ChartLayoutTests.cpp
      └─ Widget/   ChartWidget.cpp · ChartCanvas.*
```

---

## 5. 核心类型速查

| 类型 | 职责 | 谁改它 |
| --- | --- | --- |
| `FChartSpec` | 配置根（Options + 两根轴 + 系列列表） | 细节面板 / `SetChartSpec` |
| `FChartOptionsSpec` | 标题、字体、配色、网格样式、背景、图例 | 同上 |
| `FChartAxisSpec` | 轴标题、尺度、域模式、刻度格式/数量、单位、刻度后缀 | 同上 |
| `FChartSeriesSpec` | SeriesKey、DisplayName、Kind、Style、显隐、值轴 | 同上 |
| `FChartSeriesStyle` | 颜色、线宽、标记、曲线、填充、发光 | 同上 |
| `FChartSeriesDataPool` | `TMap<FName, FChartSeriesBuffer>`，按 Key 对齐数据 | `SetSeriesPoints` 等 |
| `FChartDomain` | 一段有效数据区间（含退化修正） | 编译器 |
| `FChartCartesianSpace` | 正/逆映射的唯一事实源 | 编译器 |
| `FChartLayout` | 绘图区、轴标签、图例条目矩形 | 布局器 |
| `FChartFrame` | 本帧全部几何（POD） | 编译器 |
| `FChartPaintContext` | 绘制出口 | 画布 |

---

## 6. 使用

### 6.1 设计期

1. 新建 Widget Blueprint → Palette 的 **NXCharts** 分类 → 拖入 **NXChartWidget**（类名 `UNXChartWidget`）
2. 给它空间（默认想要 480×320；放 Canvas Panel 里要手动铺，VerticalBox/SizeBox 里自动铺满）
3. 在 Details 里展开 `Spec` 配置外观

**字体留空即可**，会回落到引擎默认字体（打包后仍有效）。

### 6.2 运行时 API（蓝图）

| 节点 | 用途 |
| --- | --- |
| `AddSeries(Key, Kind, Color, Points, DisplayName)` | **最常用**：建配置 + 灌数据，一步到位（upsert） |
| `AddOrUpdateSeries(SeriesSpec)` | 需要精细控制（填充/发光/曲线）时用 |
| `RemoveSeries(Key)` | 删除，连带数据 |
| `SetSeriesVisible(Key, bVisible)` | 显隐 |
| `SetSeriesStyle(Key, Style)` / `SetSeriesColor(Key, Color)` | 改样式 |
| `SetSeriesPoints(Key, Points)` | 整体替换数据 |
| `AddSeriesPoint(Key, Point)` | 单点追加（实时） |
| `ClearSeries(Key)` | 清空数据，保留配置 |
| `SetChartSpec(Spec)` / `SetAxisSpec(Axis, AxisSpec)` | 整体换配置 |
| `SetDomainOverride(XRange, YRange)` / `ClearDomainOverride()` | 手动指定/清除显示范围 |
| `OnChartHovered(SeriesKey, PointIndex, DataValue)` | 悬停事件（只在最近点变化时触发） |

`SeriesCapacity`（Details 里）> 0 时，每条系列只保留最近 N 个点，适合滑动窗口。

### 6.3 控件自带交互

| 操作 | 效果 |
| --- | --- |
| 鼠标移入绘图区 | 竖直十字准线 + 各系列最近点高亮 + 提示框 |
| 滚轮 | 以光标为锚点缩放（锚点在屏幕上不动） |
| 中键拖拽 | 平移（以按下瞬间的坐标系为基准，无累积误差） |
| 左键点图例 | 切换该系列显隐（隐藏后条目变暗，仍可点回来） |

### 6.4 数据接入

三种节奏：

```text
一次性（切页/后端下发）   SetSeriesPoints
实时追加（波形/监控）      AddSeriesPoint + SeriesCapacity
批量（网络包/批量帧）      C++ 侧 AddSeriesPoints(TArrayView)
```

**线程约束**：Widget API 必须在 GameThread 调用。网络回调在其他线程时，先入队，再在游戏线程统一刷。

### 6.5 复刻"深色渐变 + 平滑曲线 + 面积填充"那张图

| 字段 | 值 |
| --- | --- |
| `Options.bGradientBackground` | ✅ |
| `Options.BackgroundColor` / `BackgroundColorBottom` | 深蓝 → 更深 |
| `Options.bDashedGrid` + `GridDashLength/GapLength` | ✅ / 4 / 4 |
| `Options.GridColor` / `AxisColor` / `LabelColor` | 紫灰 / 亮青 / 白 |
| `AxisX.TickSuffix` | `s` |
| `AxisY.DesiredTickCount` | `4`（0~1500 → 步长 500） |
| `Series[].Style.Curve` | `Smooth` |
| `Series[].Style.bFillArea` + `AreaFillOpacity` | ✅ / 0.4 |
| `Series[].Style.bGlow` + `GlowThickness` | ✅ / 8 |
| `Series[].DisplayName` | `单位m/s` |

---

## 7. 扩展指南

### 7.1 新增一种图表类型（如柱状图）

1. 在 `FChartGeometry` 里加顶点/矩形容器（如果不用折线表达）
2. `FChartFrameBuilder::BuildSeriesGeometry` 里按 `Kind` 分支产出几何
3. `SChartCanvas::PaintSeries` 里按 `Kind` 分支绘制
4. 若是分类数据，先在 `Layout/ChartSpace.h` 里加 `Band` 尺度的映射，再让轴域解析走它

> 关键：**不要把新图表的逻辑塞进 `ChartWidget`**。它只负责"什么时候重建"。

### 7.2 新增一种坐标/尺度（极坐标、对数轴）

- 在 `FChartScaleType` 加枚举值
- 在 `FChartCartesianSpace` 里把 `DataXToLocal` 改成按尺度分派（或引入 `IChartScale` 接口）
- 布局层不用动（它只关心文字尺寸与矩形）

### 7.3 新增一种图元（多边形、弧、图片）

- 在 `FChartPaintContext` 加方法，内部转成已有的 Slate 调用
- 需要填充时参考 `DrawGradientBand`：它演示了 `MakeCustomVerts` 的完整用法（顶点零初始化、窗口空间坐标、UV 取资源代理中心）
- **不要在业务代码里直接调 Slate API**，否则以后换后端要改一片

### 7.4 新增交互（框选、双击复位、右键菜单）

- 判定放在 `SChartCanvas`，坐标反查用 `Frame.Space.LocalToData`
- 需要改显示范围时调 `Widget->SetDomainOverrideRaw()`，**不要改数据**
- 需要通知外部就走 `UNXChartWidget` 上的动态多播委托（参考 `OnChartHovered`）

### 7.5 换肤

- 短期：直接改 `FChartOptionsSpec` 的各个颜色字段
- 长期：让 `FChartLayoutBuilder` 在解析字体/颜色时先读 `UNXChartTheme`、再被 Spec 覆盖（`UNXChartTheme` 目前是预留入口，未接入渲染）

---

## 8. 性能与约束

| 约束 | 数值 | 位置 |
| --- | --- | --- |
| 抽稀触发阈值 | 点数 > 绘图区宽度 × 2 | `ChartFrameBuilder` |
| 抽稀输出上界 | ≈ 绘图区宽度 × 2 | `ChartDecimator` |
| 曲线细分源点上界 | 4000（超过则退化为折线） | `ChartCurve` |
| 曲线细分输出上界 | 20000 | `ChartFrameBuilder` |
| 每段采样数 | 长度 / 6 像素，夹在 [2, 16] | `ChartCurve` |

**绘制成本 ≈ 绘图区宽度**，与数据量基本无关。这是 10 万点能跑的前提。

---

## 9. 已知限制

| 限制 | 说明 |
| --- | --- |
| `EChartSeriesKind` 不影响绘制 | `Bar` / `Scatter` / `Area` 目前都按折线画；散点可用 `Line` + `bShowMarkers` 近似 |
| 图例不换行 | 系列多时会画到控件外 |
| 无数据源对象 | 只有推模型 API，没有 `UChartDataSource` 拉模型 |
| 十字准线只有竖线 | 没有横线 |
| Y 轴标题横排 | 放在绘图区上方，未做 -90° 旋转 |
| `UNXChartTheme` 未接入 | 配色实际来自 `FChartOptionsSpec` |
| 运行时系列不持久化 | 活在 Widget 实例上，重建界面会回到资产初始值 |

---

## 10. 构建与测试

### 构建

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' `
  MyProject2Editor Win64 Development `
  -Project="D:\WorkSpace\GitLab\DigitalToolPlugin\MyProject2.uproject" -WaitMutex
```

### UE 5.8 升级注意

`Target.cs` 必须是 `DefaultBuildSettings = BuildSettingsVersion.V7;`。
用 V6 会让 UBT 在生成 makefile 阶段直接拒绝：

```
MyProject2Editor modifies the values of properties:
[ UnreachableCodeWarningLevel: Off != Error, ... ]
This is not allowed, as MyProject2Editor has build products in common with UnrealEditor.
```

### 自动化测试（10 组）

```
NXCharts.Frame.Pipeline        2 条系列 → 协调成 3 条 → 增量重建 → 域扩张整帧重建
NXCharts.Frame.Decimate        1 万点压到 ≤ 440 顶点，索引升序、覆盖头尾
NXCharts.Frame.Curve           单调插值无过冲、X 保持单调、阶梯点数、乱序退化
NXCharts.Frame.StyleFlags      填充/发光/曲线标志透传到几何
NXCharts.Frame.DomainOverride  优先级：Override > Manual > Auto
NXCharts.Layout.Domain         域纳入、NaN 忽略、退化修正、Pad
NXCharts.Layout.Ticks          1-2-5 步长、小数位、格式化、域外刻度裁剪
NXCharts.Layout.Space          中点映射、逆映射往返、Y 轴翻转
NXCharts.Layout.LabelFormat    刻度后缀、DisplayName 回退
NXCharts.Layout.Legend         隐藏系列保留、条目顺序、不重叠
```

命令行跑（**必须先关闭编辑器**，否则第二实例会被顶掉、测试不执行）：

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'D:\WorkSpace\GitLab\DigitalToolPlugin\MyProject2.uproject' `
  '-ExecCmds="Automation RunTests NXCharts;Automation Quit"' `
  -unattended -nopause -nosplash -nosound -nop4 -windowed -ResX=640 -ResY=480
```

> **不要加 `-nullrhi`**：在 5.7/5.8 上都会让编辑器在 `FEngineLoop::Init()` 之后
> `Fatal error: Null assigned to TNotNull` 崩溃，测试一条都跑不到。

跑完队列会打印 `Automation Test Queue Empty 10 tests performed`，但进程不会自己退出，需要手动结束。
