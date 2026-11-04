# NXCharts 阶段 A 实装状态

> 对照 `NXCharts_Architecture_V2.md` 的落地记录
> 更新：2026-11-03 · 引擎：**UE 5.8**（项目已从 5.7 升级）
> · 编译验证：`Build.bat MyProject2Editor Win64 Development` → `Result: Succeeded`
> · 自动化测试：`Frame.Pipeline` / `Frame.Decimate` / `Frame.Curve` / `Frame.StyleFlags` / `Frame.DomainOverride` / `Layout.Domain` / `Layout.Space` / `Layout.Ticks` / `Layout.LabelFormat` / `Layout.Legend` **10/10 Success**（UE 5.8 上实测）

---

## 1. 一句话

**"运行时 2→3 条折线"已经闭环**：不再需要 UMG 资产，两个蓝图节点即可。
配置、数据、布局、绘制四层已经分层落地；交互与性能优化按计划后置。

---

## 2. 当前目录（26 个源文件）

```
Source/NXCharts/
├─ NXCharts.Build.cs              Slate/SlateCore/Engine 已改为 Public 依赖
├─ Public/
│  ├─ NXCharts.h
│  ├─ Core/
│  │  ├─ NXChartsTypes.h          公共枚举（SeriesKind / Axis / Scale / Layer / 对齐）
│  │  ├─ ChartPaintContext.h      绘制收敛层：线/虚线/矩形/描边/圆/十字/文字/裁剪
│  │  └─ ChartTheme.h             DataAsset（Spec 通过 TSoftObjectPtr 引用）
│  ├─ Spec/
│  │  ├─ ChartSpecTypes.h         FChartSeriesStyle / FChartSeriesSpec / FChartAxisSpec
│  │  └─ ChartSpec.h              FChartSpec / FChartOptionsSpec / FChartDiagnostics
│  ├─ Data/
│  │  └─ ChartSeriesBuffer.h      FChartSeriesBuffer（含 Capacity 窗口）+ FChartSeriesDataPool
│  ├─ Layout/
│  │  ├─ ChartMath.h              数学工具（当前未被引用，保留给命中测试/抽稀）
│  │  ├─ ChartSpace.h             FChartDomain / FChartCartesianSpace（正+逆映射）
│  │  ├─ ChartTicks.h             FChartTick / FChartTickSet / 1-2-5 刻度
│  │  └─ ChartLayout.h            FChartLayout / FChartLayoutBuilder（文字实测驱动布局）
│  ├─ Frame/
│  │  └─ ChartFrame.h             FChartFrame / FChartSeriesGeometry（POD 帧）
│  └─ Widget/
│     └─ ChartWidget.h            UChartWidget（唯一对外入口）
└─ Private/
   ├─ Core/  ChartPaintContext.cpp · ChartTheme.cpp
   ├─ Data/  ChartSeriesBuffer.cpp
   ├─ Layout/ ChartMath.cpp · ChartSpace.cpp · ChartTicks.cpp · ChartLayout.cpp
   ├─ Frame/ ChartFrameBuilder.h/.cpp
   ├─ Tests/ ChartLayoutTests.cpp    Domain / Ticks / Space 三组自动化测试
   └─ Widget/ ChartWidget.cpp · ChartCanvas.h/.cpp
```

**已删除**（被 Spec + Frame 取代）：`Feature/Modules/*`、`Feature/Components/*`、`Core/ChartGlobalOption.h`。

---

## 3. 蓝图用法：2 条线 → 3 条线（零打包）

```
// 新增/更新一条系列（存在则改配置，不存在则追加）
AddOrUpdateSeries(
    SeriesKey = "cpu",
    Kind      = Line,
    Style.Color = (0.2, 0.6, 1.0),  LineThickness = 2.0,
    bVisible  = true)

// 灌数据（可晚到、可每帧追加）
SetSeriesPoints("cpu", Points)      // 整体替换
AddSeriesPoint("cpu", (X, Y))       // 实时追加

// 删除
RemoveSeries("cpu")

// 显隐（图例/复选框最常用）
SetSeriesVisible("cpu", false)
```

不需要碰 UMG，不需要重新编译，不需要重开界面。

设计期仍然可以配置：`UChartWidget::Spec` 是普通结构体属性，在 Details 面板里直接展开编辑
（比原来的 `Instanced` 子对象数组更好用：可复制、可整组改、可存盘、可从后端下发）。

---

## 4. 已经落地的架构约束

| 约束 | 实现位置 |
| --- | --- |
| 资产里只存值，不存对象图 | `UChartWidget::Spec`（`FChartSpec`） |
| 数据与配置分离，数据不进 Spec | `FChartSeriesDataPool` + `FChartSeriesBuffer` |
| 绘制只读快照 | `SChartCanvas::OnPaint` 只读 `FChartFrame` |
| 帧按脏标重建 | `UChartWidget::GetFrame` 比较 `SpecRevision` / 数据版本和 / 控件尺寸 |
| 唯一坐标事实源 | `FChartCartesianSpace::DataToLocal` / `LocalToData` |
| 布局由文字实测驱动 | `FChartLayoutBuilder`（`FSlateFontMeasure`） |
| 曲线裁剪在绘图区内 | `FChartPaintContext::PushClip/PopClip` |
| 轴/网格/图例/标题由帧生成 | 不再是用户装配的对象 |
| 配色与网格样式来自 Spec | `FChartOptionsSpec` 的背景/网格/轴线/标签/标题/图例色 + 虚线开关；画布不再持有硬编码颜色 |
| 图例文案与绑定解耦 | `FChartSeriesSpec.DisplayName`（FText）优先，留空回退 `SeriesKey` |
| 曲线形状来自 Spec | `FChartSeriesStyle.Curve`（Linear / Smooth / Step）；Smooth 为单调三次插值，**先抽稀再细分**，且限制在 4000 源点以内 |
| 渐变填充用自定义顶点 | `FChartPaintContext::DrawGradientBand` 用 `MakeCustomVerts` + 顶点色插值；顶点是窗口空间坐标，内部做 `LocalToAbsolute` |
| 图例的"画"和"点"共用同一份矩形 | 布局阶段产出 `TArray<FChartLegendItem>`（条目矩形 + 色块矩形），画布只消费它，命中测试也只查它 |

---

## 5. 与设想的偏差（说明）

1. **没有做 `UChartSeriesRenderer` 渲染器池。**
   渲染器被证明是多余的一层：几何由 `FChartFrameBuilder` 直接产出，
   `UChartWidget` → `FChartFrame` → `SChartCanvas` 之间不需要 UObject 中转。
   将来若要做「蓝图自定义图表类型」，应该在 `Kind → 几何构建函数` 上开注册表，比 UObject 渲染器更轻。

   分档失效则已经实现（`FChartFrameBuilder::ReconcileData`）：
   - Spec / 尺寸变化 → `Build()` 整帧重建（域 → 刻度 → 布局 → 全部几何）；
   - 只有数据变化且**域没变** → 只重建 `Revision` 变化的那几条系列几何，布局与文字测量全部跳过；
   - 数据变化把自动域撑开 → 退化为整帧重建（刻度、标签、绘图区都会变，这是正确行为）。

2. **`UChartWidget` 没有做逐系列的点级增量**（例如只重算折线尾部）。
   当前是「该系列整体重建顶点」。抽稀落地后会换成按像素列重建，届时一并解决。

2. **没有做 `UChartDataSource`**（拉模型）。当前只有推模型 API；网络线程批量入队留到阶段 B。

3. **抽稀已实现**（`Private/Frame/ChartDecimator.*`）：按像素列做 min/max 分桶，
   直接映射前先选点，顶点数上界从 O(数据量) 变成 O(绘图区宽度)；
   保留的一定是真实数据点，所以 tooltip 取到的是真值。数据量 ≤ 阈值时自动不抽稀。

4. **交互已全部接入**：
   - 悬停：十字准线 + 各系列最近点高亮 + 内置提示框（表头是悬停处的 X 值，每行是
     `SeriesKey + Y 值`，精度跟随轴刻度步长），并通过 `OnChartHovered` 广播给蓝图
     （只在最近点变化时触发）。
   - 滚轮缩放：以光标所在数据值为锚点，锚点在屏幕上保持不动。
   - 中键拖拽平移：以按下瞬间的坐标系为基准计算位移，避免累积误差。
   - 缩放/平移改的是**运行时域覆盖**（`FChartDomainOverride`），不动数据；
     优先级是 `Override > AxisSpec.ManualDomain > 自动推导`，蓝图调 `ClearDomainOverride()`
     即可回到自动域。

5. **Y 轴标题横向排在绘图区上方**，未做 -90° 旋转（避免引入 render transform 与额外风险）。

---

## 6. 如何在编辑器里验收

1. 打开工程（首次会自动加载插件），新建一个 Widget Blueprint。
2. 在 Palette 的 **NXCharts** 分类下拖入 `ChartWidget`（`GetPaletteCategory` 已重写）。
3. Details 里展开 `Spec`：
   - `Options.Title` 填 "Demo"
   - `AxisY.Unit` 填 "rpm"
   - `Series` 加两条：`SeriesKey` = `A` / `B`
4. 蓝图 `Event Construct`：

```
SetSeriesPoints("A", 一组点)
SetSeriesPoints("B", 一组点)
```

5. 运行，应看到：标题、Y 轴标题、网格、刻度标签、两条折线、底部图例。
6. 运行中调用 `AddOrUpdateSeries("C", ...)` + `SetSeriesPoints("C", ...)` → **立刻多一条线**，无闪烁。
7. 调用 `RemoveSeries("C")` → 回到两条。
8. 鼠标移进绘图区 → 应出现竖直十字准线、各系列最近点高亮、跟随光标的提示框；
   绑 `OnChartHovered` 可以在蓝图里看到 `SeriesKey / PointIndex / DataValue`。
9. 在绘图区滚轮 → 以光标为锚点缩放；中键拖拽 → 平移；蓝图调 `ClearDomainOverride()` → 回到自动域。

> **跑测试前请先关掉编辑器。** 同一个项目再起一个 `UnrealEditor-Cmd` 会与已开的实例冲突
>（日志里能看到它加载的是 `UnrealEditor-NXCharts-0001.dll` 这种热重载副本），
> 表现是队列 Ready 之后编辑器和测试一起退场、`Test Completed` 一行都没有。
> 关掉编辑器重跑即可。

> 自动化测试可以直接从命令行跑（本机已实测 10/10 通过）：
>
> ```powershell
> & 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
>   'D:\WorkSpace\GitLab\DigitalToolPlugin\MyProject2.uproject' `
>   '-ExecCmds="Automation RunTests NXCharts;Automation Quit"' `
>   -unattended -nopause -nosplash -nosound -nop4 -windowed -ResX=640 -ResY=480
> ```
>
> **坑：不要加 `-nullrhi`。** 这台机器上加了它，编辑器会在 `FEngineLoop::Init()` 之后
> 立刻 `Fatal error: Null assigned to TNotNull` 崩溃（调用栈只有 Core/Engine，与插件无关），
> 测试一条都跑不到。不加则正常执行，跑完队列会打印 `Automation Test Queue Empty 6 tests performed`，
> 但进程不会自己退出，需要手动结束。

---

## 7. UE 5.8 升级注意（踩过的坑）

从 5.7 升到 5.8 后，第一次编译会在 UBT 阶段直接失败，报：

```
MyProject2Editor modifies the values of properties:
[ UnreachableCodeWarningLevel: Off != Error, ReturnTypeWarningLevel: Off != Error, DanglingWarningLevel: Off != Error ]
This is not allowed, as MyProject2Editor has build products in common with UnrealEditor.
```

原因：`Target.cs` 里还是 `DefaultBuildSettings = BuildSettingsVersion.V6`，UBT 按向后兼容把这几项 warning level 降成 `Off`，
而 5.8 的默认值和 UnrealEditor 共用的构建环境要求 `Error`，两者冲突就拒绝生成 makefile。

修法（两个 Target.cs 都要改）：

```csharp
DefaultBuildSettings = BuildSettingsVersion.V7;
IncludeOrderVersion  = EngineIncludeOrderVersion.Unreal5_7;   // 暂不升 5_8，避免 include 顺序变化
```

改完插件代码**一行都不用动**，5.8 下直接编译通过、9/9 测试通过。

> 另一种修法是 `BuildEnvironment = TargetBuildEnvironment.Unique;`，但那会多编一套构建环境（更慢、产物更大），不推荐。

---

## 8. 下一步（阶段 B）

| 优先级 | 事项 | 说明 |
| --- | --- | --- |
| P0 | 柱状图 / 分类轴（Band scale） | `Kind` 目前不影响绘制；柱状图会倒逼 `FChartScale` 抽象 |
| P1 | `UChartDataSource` + 网络线程入队合并 | 数字孪生数据源接入（拉模型 / 跨线程） |
| P1 | 逐点增量（只重算折线尾部） | 实时追加场景下进一步省掉整条线的重建 |
| P2 | 图例换行 | 系列很多时图例是单行/单列，会画到控件外，需要折行 |
| P2 | 清理 `Layout/ChartMath.h` | 目前无调用者：要么删，要么把 `PointToSegmentDistance` 用于正交命中 |
