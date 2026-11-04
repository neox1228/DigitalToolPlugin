# NXCharts 架构 V2：运行时可配置的图表系统

> 触发需求：图表不能只在 UMG 里配死。运行时数据从 2 条折线变成 3 条，应当能在程序里改，而不是回 UMG 重新配置、重新打包。
> 前置：`NXCharts_Architecture_Review.md`（现状评审）、`NXCharts_Development.md`（原始实现说明）
> 日期：2026-10-30

---

## 1. 需求拆解：运行时到底要改什么

"运行时可改"不是一个开关，它至少有七件事，复杂度完全不同：

| # | 运行时可变项 | 频率 | 归属 |
| --- | --- | --- | --- |
| R1 | 数据点（值变化） | 高频（1–60 Hz） | 数据通道 |
| R2 | **系列数量**（2 → 3 条） | 中低频（切页/切指标时） | 配置通道 |
| R3 | 系列样式（颜色/线宽/标记） | 低频 | 配置通道 |
| R4 | 系列顺序 / 显隐 | 低频（图例点击） | 配置通道 |
| R5 | 轴配置（范围/刻度/单位/双 Y 轴） | 低频 | 配置通道 |
| R6 | 标题 / 图例位置 / 主题 / 内边距 | 低频 | 配置通道 |
| R7 | 图表整体换一套配置（后端下发） | 低频 | 配置通道 |

**关键判断：R2–R7 全部属于"配置"，只有 R1 属于"数据"。** 而现在的架构把 R2–R7 存在
`Instanced` UObject 里——那是 Widget BP 资产的一部分。这就是"必须重新打包"的根因。

> 假设：本文将"系统界面"理解为**程序内 UI（大屏上的下拉框/复选框/配置面板）或蓝图逻辑驱动的运行时修改**。
> 如果你的场景是"要让操作员在一个内建的图表编辑面板里所见即所得地拖拽配置"，
> 那本文的 `FChartSpec` 正是那个面板要读写的数据结构——面板本身是额外的 UI 工作量，不影响架构。

---

## 2. 现有架构的可行性判定

先给结论：**机制有一半，状态模型完全没有。** 能硬拼出来，但会长成一堆补丁。

| 运行时需求 | 现有架构能做到吗 | 判定 |
| --- | --- | --- |
| R1 改数据 | 能。但 `TArray<FVector2D>` 是 UPROPERTY，BP 赋值是**整体深拷贝**，且每次都要手动 `RefreshChart` | ⚠️ 能跑，10k 点以上会疼 |
| R2 加系列 | 有 `AddComponent`，内部 `NewObject` 可行 | ⚠️ 机制在，但没有 Key、没有校验、没有批量 |
| R2 删系列 | **不存在 `RemoveComponent`** | ❌ 结构性缺失 |
| R2 改系列顺序 | 数组可重排，但没有语义，且无法感知变化 | ❌ |
| R3 改样式 | 属性可写，但没有变更钩子，不会触发重绘 | ❌ |
| R4 显隐 | `bVisible` 存在，但改完不会 invalidate | ⚠️ |
| R5 改轴范围 | `XAxisRange` / `YAxisRange` **没有 UPROPERTY**，蓝图根本改不了 | ❌ |
| R7 后端下发配置 | 没有一个值类型能承载整套配置；Instanced 子对象无法从 JSON/DataTable 反序列化 | ❌ |
| 批量应用（一次加 3 条） | `AddComponent` 每次都内部 `RefreshChart()`，中间态会被画出来 | ⚠️ |
| 配置存盘 / 回滚 / 复制到另一张图 | Instanced 子对象绑在 BP 资产上，运行时新建的不会存 | ❌ |

再加两条运行时特有的隐患：

- `AddComponent(Class, ModuleName)` 只写 `ParentModuleName`，**不校验模块是否存在**。先加系列、
  后加坐标模块（或模块改名）→ 系列静默不画，没有任何报错。
- 运行时新建的 UObject 如果 Outer 是 Widget，会和 BP 里已有的 Instanced 子对象**形成两个事实源**：
  重开界面时，资产里的 2 条会回来，你运行时加的 1 条会消失。

**所以：现有架构不是"改改就能用"，而是"权威状态的存放位置错了"。**

---

## 3. 根因：三个身份被混在一起

### 3.1 配置 = 资产

`Instanced + EditInlineNew` 的 UObject 是**为设计期作者内容**设计的机制：它序列化进 Widget BP，
在 Details 面板里给你嵌套编辑。它对"资产"是完美工具，对"运行时状态"是错误工具。

你现在想要的恰恰相反：**系列集合应该是数据，不是资产。** 只要它是资产，改它就必须重新打包。
这不是 API 缺失，是类型选错了。

> 一个直接的推论：**"删系列"之所以不存在，是因为在资产模型里"删"= 在设计器里删对象，
> 根本不需要运行时 API。** 缺 `RemoveComponent` 是架构的症状，不是遗漏。

### 3.2 轴/图例/网格被当成"组件"

现在要画一张图，用户得配 N 个对象并手工配对：

```
CartesianCoordModule(Name=MainCoord)
  + AxisComponent(ParentModuleName=MainCoord, AxisType=X)   ← 一条轴一个对象
  + AxisComponent(ParentModuleName=MainCoord, AxisType=Y)   ← 再一个对象
  + UlineSeriesComponent(ParentModuleName=MainCoord)
```

**但轴、网格、图例、标题都不是"系列"，它们是坐标系的派生物。** 它们应该由图表根据配置自动生成，
而不是让用户手工装配。现在这个模型导致：每加一条线，配置成本是"3 个对象 + 2 处字符串配对"。

而且 [AxisComponent.cpp](D:/WorkSpace/GitLab/DigitalToolPlugin/Plugins/NXCharts/Source/NXCharts/Private/Feature/Components/AxisComponent.cpp)
的存在本身就是为了绕开"坐标系模块不知道自己要画轴"这件事——两个对象要互相找到对方，所以有了
`ParentModuleName`。这是自我制造的耦合。

### 3.3 数据与配置同一个通道

`DataPoints` 挂在系列 UObject 上，于是：
- 改一个点 → 触发 UObject 属性系统 → 全量重绘；
- 想加一条线 → 必须 NewObject 一个"带数据和样式的对象"，而数据本可以晚到、可以复用。

**配置是低频的、要序列化的、要 diff 的；数据是高频的、要增量追加的、不需要序列化的。**
两者必须走不同通道，否则高频路径会被低频机制拖死。

---

## 4. 架构 V2 总览

核心思想一句话：**把"图表长什么样"从 UObject 资产结构，降级成一个可序列化的值类型（Spec）；
UObject 只作为 Spec 的编译产物存在。**

```
┌──────────────────────────── 配置通道（低频） ────────────────────────────┐
│  FChartSpec  (USTRUCT, 纯值, 可序列化/可下发/可 diff/可存盘)            │
│    ├ FChartOptionsSpec   title / legend / theme / padding / font         │
│    ├ FChartAxisSpec[2]   X,Y: scale / domain / ticks / format / 关联轴   │
│    └ FChartSeriesSpec[]  { SeriesKey, Kind, Style, RendererClass? }      │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │  ApplySpec()  → Reconcile(diff)
┌────────────────────────── 数据通道（高频） ──────────────────┐
│  FChartSeriesBuffer  TMap<FName, { TArray<FVector2d>, Rev }> │
│  SetSeriesPoints / AppendPoints / RingBuffer / DataSource    │
└────────────────────────────────────┬─────────────────────────┘
                                     │  MarkDirty(级别)
                                     ▼
┌────────────────────────── 编译层（脏标驱动） ────────────────────────────┐
│  FChartFrame (POD，非 UObject，OnPaint 只读)                            │
│    ├ FChartLayout     plotRect / 各分区 / 刻度 / 标签实测尺寸            │
│    ├ FChartCartesianSpace  DataToLocal / LocalToData（唯一事实源）       │
│    ├ FChartSeriesGeometry[]  已映射到局部坐标、已抽稀的顶点/矩形         │
│    └ FChartDiagnostics  ApplySpec 的校验报告（给蓝图看）                │
└────────────────────────────────────┬────────────────────────────────────┘
                                     ▼
┌────────────────────────── 渲染层 ────────────────────────────────────────┐
│  SChartCanvas::OnPaint  →  FChartPaintContext（线/矩形/文字/圆/裁剪/分层）│
│  UChartSeriesRenderer 池（原 UChartComponent，Transient，按 Spec 增删复用）│
└──────────────────────────────────────────────────────────────────────────┘
```

三条铁律（V2 的验收标准）：

1. **资产里只存值，不存对象图。** Widget BP 里只有 `FChartSpec` 这一个结构体属性。
2. **轴/网格/图例/标题由 Frame 生成，不是用户装配的对象。** 组件列表只剩"系列渲染器"。
3. **OnPaint 里不出现任何 O(数据点) 的计算，也不 NewObject。**

---

## 5. 核心类型设计

### 5.1 配置：FChartSpec

```cpp
USTRUCT(BlueprintType)
struct FChartSeriesSpec
{
    GENERATED_BODY()

    /** 稳定标识。运行时增删改查的唯一句柄，**不是数组下标** */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SeriesKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EChartSeriesKind Kind = EChartSeriesKind::Line;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FChartSeriesStyle Style;

    /** 显隐（图例点击只改这个，不重建任何东西） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bVisible = true;

    /** 高级：覆盖默认渲染器（保留蓝图子类化扩展能力） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UChartSeriesRenderer> RendererOverride;
};

USTRUCT(BlueprintType)
struct FChartAxisSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EChartScaleType Scale = EChartScaleType::Linear;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EChartDomainMode DomainMode = EChartDomainMode::Auto;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="DomainMode==EChartDomainMode::Manual"))
    FVector2D ManualDomain = FVector2D(0.0, 100.0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EChartTickFormat Format = EChartTickFormat::Auto;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Unit;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Title;
};

USTRUCT(BlueprintType)
struct FChartSpec
{
    GENERATED_BODY()

    /** 结构体版本，用于存盘/下发的向后兼容升级 */
    UPROPERTY() int32 Version = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FChartOptionsSpec Options;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FChartAxisSpec AxisX;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FChartAxisSpec AxisY;
    /** 双 Y 轴：这里可以再放开成数组，但绘图区仍然只有一个 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FChartSeriesSpec> Series;
};
```

为什么这样就解决了"重新打包"：

- 它是 **USTRUCT**，不是一个 UObject 图 → 可以 `SaveGame`、可以塞进 `DataTable`、
  可以从后端 JSON 反序列化、可以 `Copy` 到另一张图。
- 它在 Details 面板里是**结构体数组** → 照样能设计期配（比 Instanced 子对象更好用：能复制粘贴、
  能整组改、能导入导出）。
- `ApplySpec` 是幂等的 → 同一份 Spec 应用 N 次结果一致，天然适合"数据驱动的配置"。

### 5.2 数据通道：与配置彻底分离

```cpp
// 高频路径：不走 UObject 属性，不走 BP 数组赋值
struct FChartSeriesBuffer
{
    TArray<FVector2d> Points;   // 或环形缓冲
    int32 Rev = 0;              // 每次变更 ++，几何缓存靠它失效
    int32 Head = INDEX_NONE;    // 环形缓冲下的有效起点
};
```

三条 API 面向不同场景：

```cpp
// 场景 A：一次性给全量（切页、后端下发）
UFUNCTION(BlueprintCallable) void SetSeriesPoints(FName Key, const TArray<FVector2d>& Points);

// 场景 B：实时追加（数字孪生）
UFUNCTION(BlueprintCallable) void AppendSeriesPoint(FName Key, FVector2d Point);   // C++ 侧走 batch 重载
void AppendSeriesPoints(FName Key, TArrayView<const FVector2d> Points);            // C++ 快路径

// 场景 C：外部数据源对象（推模型 / 拉模型）
UCLASS(Abstract, Blueprintable)
class UChartDataSource : public UObject
{
    GENERATED_BODY()
public:
    /** 由图表按固定频率（默认 10Hz，可配）回调，不是每帧 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void PollSeries(FName Key, TArray<FVector2d>& OutPoints);
};
```

要点：

- **数据不进 Spec**。Spec 只说"有哪几条线"，数据按 `SeriesKey` 对齐。这样 `ApplySpec` 可以随时
  重放而不影响数据，数据也可以先于配置到达。
- BP 侧传 `TArray` 仍有拷贝成本，所以**高频场景给 C++ 重载 + DataSource 对象**，BP 只走低频。

### 5.3 帧对象：FChartFrame（POD，OnPaint 只读）

```cpp
struct FChartSeriesGeometry
{
    FName Key;
    EChartSeriesKind Kind;
    TArray<FVector2f> LinePoints;   // 已是局部坐标，直接喂 MakeLines，零转换
    TArray<FVector2f> Markers;
    TArray<FChartBarRect> Bars;
    FBox2D DataBounds;
    int32 DataRev = INDEX_NONE;     // 缓存校验用
};

struct FChartFrame
{
    FChartLayout Layout;            // plotRect、各轴刻度、标签尺寸
    FChartCartesianSpace Space;     // DataToLocal / LocalToData
    TArray<FChartSeriesGeometry> Series;
    FChartSpec Spec;                // 本帧的配置快照
    bool IsValid() const;
};
```

`OnPaint` 变成纯粹的"读 Frame + 调绘制"：

```cpp
int32 SChartCanvas::OnPaint(...) const
{
    if (!Owner.IsValid()) return LayerId;
    const FChartFrame& Frame = Owner->GetFrame(AllottedGeometry.GetLocalSize()); // 只在脏了才重编译
    FChartPaintContext Ctx(OutDrawElements, AllottedGeometry, LayerId);

    Ctx.PushClip(Frame.Layout.PlotRect);          // ← 修复曲线越界
    for (const FChartSeriesGeometry& G : Frame.Series) { /* 按 Kind 派发到渲染器 */ }
    Ctx.PopClip();
    return Ctx.CurrentLayer();
}
```

### 5.4 坐标与尺度：摆脱 `FVector2D` 万能数据

```cpp
class FChartCartesianSpace
{
public:
    FVector2f DataToLocal(const FVector2d& Data) const;
    FVector2d LocalToData(const FVector2f& Local) const;   // ← 逆映射，tooltip/缩放锚点用它
    const FBox2D& GetPlotRect() const;
private:
    FChartDomain XDomain, YDomain;
    FBox2D PlotRect;
};
```

`FChartScale` 从第一天就存在（Linear / Band / Time），否则柱状图的分类轴会把
"域就是两个 double"这个假设打碎——上一份评审里说的"柱状图会倒逼抽象"，指的就是这里。

---

## 6. Reconcile：diff 而不是 rebuild

这是 V2 唯一有分量的算法，也是它和"直接重建"的分水岭。

```
ApplySpec(NewSpec):
  ── 阶段 1：校验（干跑，不改状态）
     SeriesKey 重复 / 为空 → 诊断，重复者丢弃保留首个
     ManualDomain min==max → 诊断，退回 Auto
     RendererOverride 非法 → 诊断，退回默认
     失败则整体放弃（原子），把诊断写进 FChartDiagnostics 供蓝图读

  ── 阶段 2：diff（按 Key 对齐）
     Added   = New - Old      → 从池取 / NewObject 渲染器
     Removed = Old - New      → 渲染器回池（不 Destroy，复用）
     Kept    = New ∩ Old      → 比 Kind / 比 Style / 比 bVisible
     OrderChanged = 交集顺序变化
     AxisChanged  = 两个 AxisSpec 对比
     OptChanged   = Options 对比

  ── 阶段 3：应用（全部完成后才算成功）
     CurrentSpec = NewSpec
     MarkDirty(累积脏标)      ← 一次
     InvalidateWidget(脏标)   ← 一次
```

脏标映射（**这是性能结构的核心**）：

| 变更 | 脏标 | 重算范围 |
| --- | --- | --- |
| 颜色 / 线宽 / 标记大小 | `Paint` | 什么都不重算，只重绘 |
| bVisible 切换 | `Geometry` | 只重算该系列几何（外加 Auto 域） |
| 数据点变化 | `Geometry` | 只重算该系列几何 |
| 增减系列（Auto 域） | `Model` | 域 → 刻度 → 布局 → 全部几何 |
| 轴配置 / 单位 / 格式化 | `Layout` | 刻度 → 布局 → 全部几何 |
| 标题 / 图例 / Padding | `Layout` | 布局 → 全部几何 |
| 控件尺寸变化 | `Layout` | 布局 → 全部几何 |

对照现在的行为：**`AddComponent` 每次都内部调 `RefreshChart()`。运行时加 3 条线 = 3 次中间态重绘，
且用户看到的是"先有 2 条、再变 3 条"的闪烁。** Reconcile 保证第 3 阶段结束前不 invalidate。

---

## 7. 运行时 API：蓝图只看得到 6 个节点

```cpp
UCLASS()
class NXCHARTS_API UChartWidget : public UWidget
{
    GENERATED_BODY()
public:
    // ---------- 设计期初始配置（序列化进 BP，同时可被运行时覆盖） ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NXCharts")
    FChartSpec Spec;

    // ---------- 运行时配置通道 ----------
    UFUNCTION(BlueprintCallable) void SetChartSpec(const FChartSpec& InSpec);              // 整体换，幂等
    UFUNCTION(BlueprintCallable) void AddOrUpdateSeries(const FChartSeriesSpec& SeriesSpec); // 单条 upsert
    UFUNCTION(BlueprintCallable) void RemoveSeries(FName Key);
    UFUNCTION(BlueprintCallable) void SetSeriesVisible(FName Key, bool bVisible);
    UFUNCTION(BlueprintCallable) void SetAxisSpec(EChartAxis Axis, const FChartAxisSpec& AxisSpec);

    // ---------- 运行时数据通道 ----------
    UFUNCTION(BlueprintCallable) void SetSeriesPoints(FName Key, const TArray<FVector2d>& Points);
    UFUNCTION(BlueprintCallable) void SetSeriesPointsById(int32 SeriesIndex, const TArray<FVector2d>& Points);

    // ---------- 诊断 ----------
    UFUNCTION(BlueprintPure) const FChartDiagnostics& GetLastApplyDiagnostics() const;
    UFUNCTION(BlueprintPure) TArray<FName> GetSeriesKeys() const;
};
```

`AddOrUpdateSeries` / `RemoveSeries` 是给下拉框/复选框用的最小面：**它们内部都只是"改 Spec 里的一个条目 + Reconcile"**，
所以和 `SetChartSpec` 走完全相同的路径，不会有两套行为。

### 2 → 3 条线的完整流程对照

**现在（必须重新打包）：**

```
UMG Designer → 打开 Widget BP → 在 Components 数组里点 + → 选 UlineSeriesComponent
  → 设 ParentModuleName = "MainCoord" → 填 DataPoints → 调 RefreshChart → 编译 → 打包
```

**V2（运行时，零打包）：**

```
蓝图：
  Event OnMetricsChanged(TArray<FMetricDef> Metrics)
    ForEach Metrics
      → Make FChartSeriesSpec { SeriesKey = Def.Key, Kind = Line, Style.Color = Def.Color }
      → ChartWidget.AddOrUpdateSeries(Spec)
    → ChartWidget.SetSeriesPoints(Def.Key, Def.Points)   ← 这一步甚至可以晚到、异步到
```

`AddOrUpdateSeries` 内部：找到 Key 存在则改 Style（走 `Paint` 脏标），不存在则从池里取一个渲染器
（走 `Model` 脏标）。**用完一次的渲染器回池，下次加线零分配。**

如果你要做"系统界面"，那界面只需要：

```
UMG: 一个 VerticalBox + 一组 CheckBox（每条指标一个）
CheckBox.OnChanged → ChartWidget.SetSeriesVisible(Key, bChecked)
```
不需要碰 Spec，也不需要知道渲染器是什么。

---

## 8. 线程与实时（数字孪生特有）

数据多半来自网络线程 / WebSocket 回调，**Spec 和 Frame 都必须在 GameThread 上改**。给两条通道：

```cpp
// 网络线程里：只入队，不碰 UObject
void UChartWidget::EnqueueSeriesPoints(FName Key, TArray<FVector2d>&& Points)
{
    FScopeLock Lock(&PendingLock);
    PendingUpdates.Add(Key, MoveTemp(Points));
}

// GameThread 每帧（或固定 10Hz）合批冲刷一次
void UChartWidget::FlushPending()
{
    if (PendingUpdates.Num() == 0) return;
    // 一次性合并所有 Key 的数据，只打一次 Geometry 脏标
    ...
}
```

这样即使后端 60 次/秒推 3 条线，也是 1 次合并重算，而不是 180 次重绘。

---

## 9. 迁移映射：旧代码怎么处置

| 现有 | V2 处置 | 说明 |
| --- | --- | --- |
| `UChartWidget.Modules` / `Components`（Instanced） | **删除** | 被 `FChartSpec` + 渲染器池取代 |
| `FChartGlobalOption` | 并入 `FChartOptionsSpec` | 顺手把 `Padding/bAntiAlias/BackgroundColor` 真正接线 |
| `UChartTheme` | 保留为 Spec 里的主题引用（`TSoftObjectPtr<UChartTheme>`） | 便于项目级换肤 |
| `UChartModule` | **大部分消失** | 坐标系 → `FChartCartesianSpace`；图例/网格/标题 → Frame 生成物 |
| `UCartesianCoordModule` | 拆解 | 域/刻度 → `FChartDomain` + `FChartTickGenerator`；直接绘制 → 删 |
| `UChartComponent` | 降级为 `UChartSeriesRenderer` | 去掉数据、去掉 `ParentModuleName`、去掉 `GetChartWidget` |
| `UAxisComponent` | **删除** | 轴由 Frame 根据 `FChartAxisSpec` 生成 |
| `UlineSeriesComponent` | → `ULineSeriesRenderer` | 只保留 `BuildGeometry` + `Paint` + `HitTest` |
| `ChartDrawContext` | 保留并扩图元 | 补 `DrawText/DrawCircle/Polygon/DashedLine` + `PushClip/PopClip` |
| `SChartCanvas` | 保留 | `OnPaint` 改为读 Frame |
| `ChartUtils` | 保留扩张 | nice-number 刻度、格式化、抽稀、clamp |

**净效果：用户要配的东西从"N 个对象 + 字符串配对"降到"1 个结构体"，
要写进资产的从对象图降到值，运行时增删系列从"不存在"变成"两个蓝图节点"。**

---

## 10. 分阶段实施

### 阶段 A：Spec + Reconcile + Frame（约 1.5 周）★ 先做这个

1. `FChartSpec` 族 + `FChartApplyReport`。
2. `ApplySpec` + Reconcile（含池化渲染器、原子应用、一次失效）。
3. `FChartFrame` + 域/刻度/布局（含 `FSlateFontMeasure` 文字实测）。
4. Frame 生成轴/网格/标题/图例骨架。
5. `SChartCanvas::OnPaint` 改读 Frame。

**验收**：蓝图循环调用 `AddOrUpdateSeries` 从 2 条变 3 条，闪烁 0 次、不需要重开界面；
`RemoveSeries` 回到 2 条；同一份 `Spec` 应用两次结果一致。

### 阶段 B：数据通道 + 几何缓存（约 1 周）

环形缓冲、脏标分级、几何缓存、按像素列抽稀（min/max）、裁剪。

**验收**：10 万点 × 3 条在拖动窗口时稳定 60fps；60Hz 追加只触发 1 次/帧重算。

### 阶段 C：移动/交互/扩展（1–2 周）

tooltip（用 `LocalToData`）、缩放平移（改域不改数据）、图例点击联动、
`RegisterSeriesRenderer`（蓝图注册自定义 Kind）、柱状图验证 band scale。

---

## 11. 风险与取舍

| 风险 | 应对 |
| --- | --- |
| Spec 结构体会随需求膨胀，改字段破坏存盘 | 加 `Version` + `UpgradeSpec()`；存盘只存 Spec |
| Reconcile 是唯一复杂点，容易出边界 bug | 它是纯 C++、POD in/out → **写单测**，这是整个插件里最适合单测的一层 |
| 失去"Designer 里拖渲染器"的灵活度 | 用 `RendererOverride` + 渲染器注册表补回，且比现在更可控 |
| BP 传 `TArray` 仍有拷贝 | 高频走 C++ 重载 / DataSource；BP 只做低频操作 |
| 一次大改，中途不能用 | 分两跳：先删"轴组件 + Instanced 配置"、保留渲染器类；再换数据通道。每跳都可编译可验证 |
| 用户已有的 Widget BP 会失效 | Content 目前为空，**零迁移成本**——这是最后的机会窗口 |

---

## 12. 一句话总结

现在的架构把"图表长什么样"存成了**对象图**，所以它天然只能设计期定死；
V2 把它存成**值（Spec）**，对象图降级为编译产物。
这一改同时拿到了四个东西：运行时增删系列、配置可序列化下发、按脏标分级的性能结构、以及唯一坐标事实源。

**代价是一次性重排约 600 行代码；收益是"运行时可配置"从不可能变成默认行为。**
