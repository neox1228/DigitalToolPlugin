# NXCharts 接口文档

> 控件类：`UNXChartWidget`（Palette 里显示为 **NXChartWidget**）
> 对应引擎：UE 5.8 · 插件：`Plugins/NXCharts`
> 与 `NXCharts_Manual.md` 的分工：**手册讲架构与用法，本文只讲接口**

---

## 0. 通用约定

| 约定 | 说明 |
| --- | --- |
| 调用线程 | **只能游戏线程**。网络回调需先入队，再在游戏线程统一刷 |
| 目标 | 所有节点都需要连一个 `NXChartWidget` 引用 |
| 静默失败 | 查不到系列、下标越界等情况**不报错**，返回值表示成败；没有异常的 `PrintString` 提示 |
| 两档刷新 | 改**配置**（Spec）→ 重新布局 + 重建几何；改**点数据** → 只重建几何，不重算布局 |
| 关键字段 | `SeriesKey` 是运行时唯一句柄，**不能为空**；为空时该系列会被静默跳过 |

---

## 1. 蓝图节点

### 1.1 系列（增 / 删 / 改）

| 节点 | 参数 | 返回 | 说明 |
| --- | --- | --- | --- |
| `AddSeries` | Key, Kind, Color, Points, DisplayName | — | **最常用**。建配置 + 灌数据一步到位；Key 已存在时是 upsert |
| `AddOrUpdateSeries` | `FChartSeriesSpec` | — | 需要精细控制（填充/发光/曲线）时用 |
| `RemoveSeries` | Key | — | 删除系列，连带丢弃其数据 |
| `SetSeriesVisible` | Key, bVisible | — | 显隐；隐藏后图例变暗但**仍可点回** |
| `SetSeriesColor` | Key, Color | — | 只改颜色 |
| `SetSeriesStyle` | Key, `FChartSeriesStyle` | — | 整块替换样式，不动数据 |
| `SetSeriesCurve` | Key, `EChartCurveType` | — | 只改连线方式（Linear / Smooth / Step） |
| `SetSeriesDisplayName` | Key, `FString` | — | 只改图例文案，不影响 `SeriesKey` 绑定 |

**`AddSeries` 的 upsert 语义**：同一个 Key 再次调用会在原有配置上更新 `Kind` / `Color` / `DisplayName` / `bVisible`，
**保留**之前通过 `SetSeriesStyle` 设过的填充、发光、曲线等字段。`Points` 为空时只建配置不灌数据。

### 1.2 点级数据（增 / 删 / 改 / 查）

| 节点 | 参数 | 返回 | 说明 |
| --- | --- | --- | --- |
| `SetSeriesPoints` | Key, `TArray<FVector2D>` | — | 整体替换 |
| `AddSeriesPoint` | Key, `FVector2D` | — | 追加到末尾（实时曲线） |
| `InsertSeriesPoint` | Key, Index, `FVector2D` | `bool` | 按索引插入；`Index == Count` 等价于追加 |
| `SetSeriesPointAt` | Key, Index, `FVector2D` | `bool` | 改单点 |
| `RemoveSeriesPointAt` | Key, Index | `bool` | 删单点 |
| `RemoveSeriesPointsInXRange` | Key, MinX, MaxX | `int32` | 删除 X 落在 `[MinX, MaxX]` 闭区间的点，返回删除数量 |
| `ClearSeries` | Key | — | 清空数据，**保留系列配置** |
| `GetSeriesPoints` | Key | `TArray<FVector2D>` | 取该系列全部点（拷贝） |
| `GetSeriesPointCount` | Key | `int32` | 点数 |
| `GetSeriesPointAt` | Key, Index, `out FVector2D` | `bool` | 取单点 |

`RemoveSeriesPointsInXRange` 适合"只保留最近 N 秒"的场景：一次扫描批量删，比循环调单点删除高效。

### 1.3 系列查询

| 节点 | 参数 | 返回 | 说明 |
| --- | --- | --- | --- |
| `HasSeries` | Key | `bool` | 系列是否存在 |
| `GetSeriesSpec` | Key | `FChartSeriesSpec` | 配置副本；不存在时返回默认结构（先用 `HasSeries` 判断） |
| `GetSeriesKeys` | — | `TArray<FName>` | 全部系列 Key，顺序与图例一致 |

### 1.4 配置与刷新

| 节点 | 参数 | 返回 | 说明 |
| --- | --- | --- | --- |
| `SetChartSpec` | `FChartSpec` | — | 整体替换配置，幂等 |
| `SetAxisSpec` | `EChartAxis`, `FChartAxisSpec` | — | 只替换某一根轴 |
| `RefreshChart` | — | — | 强制重建重绘；一般**不需要**手动调用 |

### 1.5 交互

| 节点 | 参数 | 返回 | 说明 |
| --- | --- | --- | --- |
| `SetCrosshairX` | `float X` | — | 用**数据坐标**的 X 定位十字准星，吸附最近点 |
| `ClearCrosshair` | — | — | 关闭外部十字准星 |
| `IsCrosshairActive` | — | `bool` | 查询是否激活 |
| `SetDomainOverride` | `FVector2D XRange`, `FVector2D YRange` | — | 手动指定显示范围（缩放/平移的底层能力） |
| `ClearDomainOverride` | — | — | 清除范围覆盖，回到 Auto / Manual |
| `HasDomainOverride` | — | `bool` | 别名：`IsViewModified` |
| `IsViewModified` | — | `bool` | 当前是否处于缩放/平移后的视图（控制外部复位按钮显隐） |

### 1.6 事件

| 事件 | 参数 | 触发时机 |
| --- | --- | --- |
| `OnChartHovered` | SeriesKey, PointIndex, DataValue | 鼠标悬停的"最近点"**变化**时（不会每帧刷） |
| `OnCrosshairSnapped` | SeriesKey, PointIndex, DataValue | 调用 `SetCrosshairX` 后，**每条系列各广播一次**吸附结果 |

---

## 2. 控件自带交互

| 操作 | 效果 |
| --- | --- |
| 鼠标移入绘图区 | 竖直十字准线 + 各系列最近点高亮 + 提示框 |
| 滚轮 | 以光标为锚点缩放（受轴的 `HardLimits` 约束） |
| **中键拖拽** | 平移（以按下瞬间的坐标系为基准，无累积误差） |
| 左键单击图例 | 切换该系列显隐 |
| **双击绘图区** | 复位视图（等价于 `ClearDomainOverride`） |

外部十字准星激活期间，鼠标悬停**让位**（不会同时出现两条竖线）。

**提示框内容**：每行 = 色块 + `DisplayName`（留空回退 `SeriesKey`）+ 数值（右对齐）。
名字过长时**自动缩小字号**适配（上限 140 slate unit，等价于 UMG 里 Text 套 SizeBox → ScaleBox），
不会把提示框撑到盖住曲线。

---

## 3. Spec 结构参考

### 3.1 `FChartSpec`

根对象，相当于 ECharts 的 `option`。**每个成员要么是一个组件块，要么是全局设置**，没有中间层。

**组件块**

| 字段 | 类型 | 对应组件 |
| --- | --- | --- |
| `Title` | `FChartTitleSpec` | 标题 |
| `Background` | `FChartBackgroundSpec` | 背景 |
| `Grid` | `FChartGridSpec` | 网格 |
| `Legend` | `FChartLegendSpec` | 图例 |
| `AxisX` / `AxisY` | `FChartAxisSpec` | 轴 |
| `Series` | `TArray<FChartSeriesSpec>` | 系列（**不含数据点**） |
| `Annotations` | `TArray<FChartAnnotationSpec>` | 注解（阈值线/色带） |

**全局设置**

| 字段 | 说明 |
| --- | --- |
| `Colors` | 跨组件共享色：`AxisColor` / `LabelColor` / `TitleColor` |
| `AxisLabelFont` | 轴标签与提示框共用；留空回落引擎默认 |
| `Padding` | 控件最外圈内边距 |
| `bAntialias` | 抗锯齿 |
| `Theme` | 主题资产（预留） |
| `Version` | 结构体版本 |
### 3.2 各配置块字段

| 块 | 字段 | 说明 |
| --- | --- | --- |
| `Title` | `Text` / `Font` | `Text` 为空则不画标题 |
| `Background` | `Color` / `bGradient` / `ColorBottom` | 纯色或垂直渐变 |
| `Grid` | `Color` / `bDashed` / `DashLength` / `GapLength` | 虚线可选 |
| `Legend` | `bShow` / `Position` / `Font` / `TextColor` | `Position`：Top / Bottom / Right |

**为什么要扁平**：早期有一个 `Options` 中间层打包这些块，导致「一级 = 一个组件」的规则被破坏。
现在所有块直接挂在 `Spec` 下，加一个块 = 加一个字段 + 一个绘制组件，`OnPaint` 不动。
### 3.3 `FChartAxisSpec`

| 字段 | 说明 |
| --- | --- |
| `AxisTitle` / `Unit` | 轴标题；`Unit` 非空时拼成 `标题 (单位)` |
| `TickSuffix` | 追加在**每个刻度标签**后，例如 `s` → `1s / 2s / 3s` |
| `Scale` | 尺度类型（当前仅 Linear 生效） |
| `DomainMode` | `Auto` 跟数据 / `Manual` 用 `ManualDomain` |
| `ManualDomain` | 手动范围 |
| `Format` / `DesiredTickCount` | 刻度格式与期望数量（实际由 1-2-5 步长决定） |
| `bShowGrid` / `bShowLabels` | |
| **`bUseHardLimits`** | 是否把取值范围限制在 `HardLimits` 内（自动域/手动域/缩放/平移全部受限） |
| **`HardLimits`** | `X` = 下界，`Y` = 上界 |

### 3.4 `FChartSeriesSpec`

| 字段 | 说明 |
| --- | --- |
| `SeriesKey` | 唯一句柄，**必填** |
| `DisplayName` | 图例文案，留空回退 `SeriesKey` |
| `Kind` | 系列类型（当前不影响绘制） |
| `Style` | 见 3.5 |
| `bVisible` | 显隐 |
| `ValueAxis` | 绑定的数值轴（双 Y 轴预留） |

### 3.5 `FChartSeriesStyle`

| 字段 | 说明 |
| --- | --- |
| `Color` / `LineThickness` / `bAntialias` | 基础样式 |
| `bShowMarkers` / `MarkerSize` | 数据点标记 |
| `bShowInLegend` | 是否出现在图例 |
| `Curve` | `Linear` / `Smooth`（单调三次插值，不过冲）/ `Step` |
| `bFillArea` / `AreaFillOpacity` | 曲线下方渐变填充 |
| `bGlow` / `GlowThickness` | 霓虹发光线 |

---

### 3.6 `FChartAnnotationSpec`

| 字段 | 说明 |
| --- | --- |
| `Kind` | `HorizontalLine`（Y = Value）/ `VerticalLine`（X = Value）/ `Band`（Value ~ RangeEnd 色带） |
| `Value` | 线的位置；`Band` 时为起点 |
| `RangeEnd` | 仅 `Band` 使用：终点 |
| `Color` / `Thickness` / `bDashed` | 样式；色带会自动降低透明度到 25% |
| `Label` | 线尾文字，留空不画 |

**绘制层**：`Annotation` —— 在网格之上、曲线之下，不遮挡数据。

## 4. 枚举

| 枚举 | 值 |
| --- | --- |
| `EChartSeriesKind` | `Line` / `Bar` / `Scatter` / `Area`（**当前都按折线绘制**） |
| `EChartCurveType` | `Linear` / `Smooth` / `Step` |
| `EChartAxis` | `X` / `Y` |
| `EChartScaleType` | `Linear` / `Band` / `Log` / `Time`（仅 Linear 生效） |
| `EChartDomainMode` | `Auto` / `Manual` |
| `EChartTickFormat` | `Auto` / `Integer` / `OneDecimal` / `TwoDecimals` / `Percent` / `Scientific` |
| `EChartLegendPosition` | `Top` / `Bottom` / `Right` |
| `EChartTextHAlign` / `EChartTextVAlign` | 文字对齐（绘制层用） |

---

## 5. C++ 接口（未暴露到蓝图）

| 接口 | 说明 |
| --- | --- |
| `GetFrame(const FVector2D& WidgetSize)` | 取本帧不可变快照 `FChartFrame`，必要时重建。**仅供绘制层** |
| `AddSeriesPoints(FName, TArrayView<const FVector2d>)` | 批量追加，避免蓝图数组参数的整体拷贝 |
| `SetDomainOverrideRaw(const FChartDomainOverride&)` | 可按单根轴覆盖（蓝图版只能同时覆盖两根） |
| `GetDomainOverride()` | 读取当前覆盖 |
| `GetCrosshairX()` | 读取外部准星的数据 X |
| `ChartQuery::FindNearestPointIndex(LocalPoints, LocalX)` | 最近点查询，鼠标悬停与外部准星共用 |

---

## 6. 行为与失败条件

| 场景 | 行为 |
| --- | --- |
| `SeriesKey` 为空 | `AddSeries` / `AddOrUpdateSeries` 直接忽略；Spec 里 Key 为空的系列**静默不画** |
| 系列不存在时改样式/改数据 | 无操作，不报错 |
| 点下标越界 | 返回 `false`，不改数据 |
| 缩放落在 `HardLimits` 之内 | **正常生效**（放大不受限制） |
| 缩放冲过 `HardLimits` | 夹到边界并应用；**再缩则忽略**该次滚轮（不回写超界覆盖值，否则反向缩放要退很多步） |
| 数据全落在 `HardLimits` 之外 | 展示整个限制区间，而不是空白 |
| `SetCrosshairX` 的 X 在视野外 | 不绘制竖线（不会画到轴外面） |
| `SetCrosshairX` 时帧尚未建好 | 不广播 `OnCrosshairSnapped`，但绘制时仍会正常显示 |
| 曲线点数 < 3 / X 乱序 / > 4000 点 | `Smooth` 静默退化成折线 |
| 改点数据 | 只重建几何，**不重算布局**；改 Spec 才会重新布局 |
| `SeriesCapacity > 0` | 下次写数据时按容量裁剪队首（一次多删一点，摊薄开销） |

---

## 7. 变更记录

### 2026-11-05（本轮）

**新增 · 轴取值范围限制**

- `FChartAxisSpec::bUseHardLimits`
- `FChartAxisSpec::HardLimits`
- 行为：自动域 / 手动域 / 缩放覆盖三条路径统一夹紧；顶到边界后缩放停止

**新增 · 交互**

- `SetCrosshairX(float)` — 外部驱动十字准星，吸附最近真实点
- `ClearCrosshair()`
- `IsCrosshairActive()`
- `OnCrosshairSnapped(SeriesKey, PointIndex, DataValue)` — 每条系列各一次
- `IsViewModified()` — 供外部复位按钮控制显隐

**新增 · 系列与点的增删改查**

- 系列查询：`HasSeries`、`GetSeriesSpec`
- 点级：`GetSeriesPoints`、`GetSeriesPointCount`、`GetSeriesPointAt`、`SetSeriesPointAt`、`InsertSeriesPoint`、`RemoveSeriesPointAt`、`RemoveSeriesPointsInXRange`

**新增 · 系列文案**

- `SetSeriesDisplayName(FName, FString)`

**新增 · 内部**

- `ChartQuery::FindNearestPointIndex` — 悬停与外部准星共用最近点算法
- `FChartSeriesBuffer` 点级操作（`GetPointAt` / `SetPointAt` / `InsertPoint` / `RemovePointAt` / `RemovePointsInXRange`）

**取消**

- 左键拖拽平移（保持中键拖拽）

**测试**

- 新增 `NXCharts.Data.PointCrud`、`NXCharts.Frame.NearestPoint`、`NXCharts.Frame.HardLimits`
- 合计 **14 组**，全部通过

**修复 · 开启 `bUseHardLimits` 后滚轮无法放大**

- 症状：缩小可以，放大没反应
- 原因：`ClampDomainToRange` 的"是否有效"判定拿夹紧后的域和**夹紧前的缩放结果**比，
  导致任何落在限制区间内的放大都被判成"已到边界"而丢弃
- 修法：判定基准改为**当前正在显示的域**；逻辑抽成 `ChartDomainUtil::ClampDomainToRange` 并补单测

---

### v0.3（2026-11）

**新增 · 绘制组件化**

- `FChartPainter`（`Private/Widget/ChartPainter.h`）：`GetLayer` / `IsEnabled` / `Paint`
- `FChartPaintArgs`：`Ctx` / `Frame` / `Widget` / `LocalSize` / `bHoverActive` / `HoverLocal`
- 六个组件各自独立文件：Background / GridAndAxes / Annotation / Series / Legend / Hover
- `EChartLayer` 新增 `Annotation` 层

**新增 · Spec 结构**

- 去掉 `FChartOptionsSpec` 中间层；`Title` / `Background` / `Grid` / `Legend` / `Colors` 直接挂在 `FChartSpec` 下
- `FChartSpec` 新增 `Annotations`（`TArray<FChartAnnotationSpec>`）
- `FChartAnnotationSpec`：`Kind` / `Value` / `RangeEnd` / `Color` / `Thickness` / `bDashed` / `Label`

**变更 · 配置来源**

| 旧 | 新 |
| --- | --- |
| `Spec`（结构体，Details 逐字段配） | `SpecPath`（相对 Content 的 JSON 路径） |

- 新增 `FChartSpecJson::LoadFromFile`（`Public/Spec/ChartSpecJson.h`）
- 新增 `ReloadSpec()` / `ReloadSpecIfChanged()` / `GetSpecFullPath()`
- 插件依赖新增 `Json` 模块

**新增 · 自动重载**

- `SChartCanvas` 可 Tick，0.5 秒轮询文件修改时间，变化即重载（只换配置，不动数据）

**测试**：14 组，全部通过
**补充 · 自动重载开关**

- `bAutoReloadSpec`（`EditAnywhere`，默认 `true`）：关闭后 `ReloadSpecIfChanged` 直接返回，画布 Tick 也跳过累加
- `ReloadSpec()` 手动重载不受该开关影响