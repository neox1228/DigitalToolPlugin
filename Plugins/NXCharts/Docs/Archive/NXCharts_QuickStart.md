# NXCharts 上手指南

> 目标读者：在 UMG 里用蓝图接图表的人。**不需要写 C++。**
> 引擎：UE 5.8 · 插件：`Plugins/NXCharts`

---

## 0. 五分钟出第一张图

1. **重启编辑器**（新 DLL 需要重新加载）。
2. 新建 **Widget Blueprint**（比如 `WBP_Chart`）。
3. 打开 Designer → 左侧 Palette → 搜 `NXCharts` → 拖入 **ChartWidget**。
4. 把它拉大（默认想要 480×320，放 Canvas Panel 里要手动铺开）。
5. 切到 **Graph** 面板 → `Event Construct`：

   ```
   ┌──────────────────────┐
   │ Event Construct      │
   └──────────┬───────────┘
              ▼
   ┌──────────────────────────────────────────────┐
   │ AddSeries                                    │
   │   Target      = (拖入的 ChartWidget 引用)     │
   │   Series Key  = A                            │
   │   Kind        = Line                         │
   │   Color       = 青色                          │
   │   Points      = (见下方"造点")                 │
   │   DisplayName = 单位m/s                       │
   └──────────────────────────────────────────────┘
   ```

6. **造点**：用一个 `For Loop (0→59)` + `Make Vector2D`，X 填索引、Y 填 `Sin` 之类的值，
   用 `Add` 节点往一个 `Vector2D` 数组里塞（数组要在 `Event Construct` 之外用 `Local Variable` 存）。

7. 播放 → 应该看到：坐标轴、刻度、网格、一条线、图例。

> 第一次配建议**先把 `Spec` 留空**，全部用 API 建。等跑通了再回细节面板微调外观，能更快定位问题出在哪。

---

## 1. 两种配置路线（可混用）

| | 细节面板 | 运行时蓝图 |
| --- | --- | --- |
| 改什么 | 图表的**初始外观** | 增删系列、改数据、改样式、改轴范围 |
| 什么时候生效 | 打开界面时 | 立刻，下一帧 |
| 需要重新打包吗 | 改了要 | **不需要** |

细节面板改的是 `Spec` 这个结构体的**初始值**；运行时 API 改的是**同一个结构体**。
两者不冲突，运行时永远覆盖设计期。

---

## 2. 运行时 API 手册

所有节点都在 **NXCharts** 分类下，Target 都是那个 `ChartWidget` 引用。

### 2.1 系列：增 / 删 / 改

| 节点 | 参数 | 用途 |
| --- | --- | --- |
| **AddSeries** | Key, Kind, Color, Points, DisplayName | **最常用**。一步建配置 + 灌数据 |
| AddOrUpdateSeries | `FChartSeriesSpec` | 需要精细控制（填充/发光/曲线）时用 |
| RemoveSeries | Key | 删掉，连带数据 |
| SetSeriesVisible | Key, bVisible | 显隐（复选框联动最常用） |
| SetSeriesColor | Key, Color | 只改颜色 |
| SetSeriesStyle | Key, `FChartSeriesStyle` | 整块换样式，不动数据 |

`AddSeries` 是 **upsert**：同一个 Key 再调一次会更新，并保留之前设过的填充/发光。

### 2.2 数据

| 节点 | 用途 |
| --- | --- |
| `SetSeriesPoints(Key, Points)` | 整体替换（切页、后端下发） |
| `AddSeriesPoint(Key, Point)` | 单点追加（实时曲线） |
| `ClearSeries(Key)` | 清空但保留系列配置 |

> 灌完数据不需要调 `RefreshChart`，会自动刷新。
> `SeriesCapacity`（细节面板里）> 0 时会自动丢弃最老的点，做滑动窗口用它。

### 2.3 坐标轴

| 节点 | 用途 |
| --- | --- |
| `SetAxisSpec(Axis, AxisSpec)` | 整体换某根轴的配置 |
| `SetDomainOverride(XRange, YRange)` | 直接指定显示范围（覆盖自动范围） |
| `ClearDomainOverride()` | 回到自动范围 |

### 2.4 事件

| 事件 | 参数 | 触发时机 |
| --- | --- | --- |
| `OnChartHovered` | SeriesKey, PointIndex, DataValue | 悬停的"最近点"变化时（不会刷） |

### 2.5 控件自带交互（不用写代码）

| 操作 | 效果 |
| --- | --- |
| 鼠标悬停绘图区 | 十字准线 + 最近点高亮 + 提示框 |
| 滚轮 | 以光标为锚点缩放 |
| **中键拖拽** | 平移 |
| **左键点图例** | 切换该系列显隐（隐藏后图例变暗，仍可点回来） |

---

## 3. 三个典型场景

### 3.1 静态监测图（一次性给全部数据）

```
Event Construct
  → AddSeries("cpu", Line, 青, 点集, "CPU 使用率")
  → AddSeries("mem", Line, 黄, 点集, "内存")
```

### 3.2 实时波形（持续追加）

```
Event Construct
  → AddSeries("wave", Line, 青, 空数组, "波形")
  → 设置 SeriesCapacity = 2000（细节面板）

Event Tick（或用 Timer，建议 10~30Hz 而不是每帧）
  → AddSeriesPoint("wave", (当前时间, 当前值))
```

### 3.3 运行时增删（2 条变 3 条，不打包）

```
Event OnMetricsChanged(Metrics)
  → ForEach Metrics
      → AddSeries(Metric.Key, Line, Metric.Color, Metric.Points, Metric.Label)

Event OnMetricsRemoved(Keys)
  → ForEach Keys → RemoveSeries(Key)
```

**没有任何一步需要回 UMG 或重新打包。**

---

## 4. 复刻那张深色渐变图

细节面板里按这个配：

| 字段 | 值 |
| --- | --- |
| `Options.bGradientBackground` | ✅ |
| `Options.BackgroundColor` | 深蓝（顶部） |
| `Options.BackgroundColorBottom` | 更深（底部） |
| `Options.bDashedGrid` | ✅ |
| `Options.GridDashLength` / `GridGapLength` | 4 / 4 |
| `Options.GridColor` | 偏紫、低透明度 |
| `Options.AxisColor` | 亮青 |
| `Options.LabelColor` | 白 |
| `Options.bShowLegend` + `LegendPosition` | ✅ / Bottom |
| `AxisX.TickSuffix` | `s` |
| `AxisY.DesiredTickCount` | `4`（0~1500 → 步长 500） |
| `Series[].Style.Curve` | `Smooth` |
| `Series[].Style.bFillArea` / `AreaFillOpacity` | ✅ / 0.4 |
| `Series[].Style.bGlow` / `GlowThickness` | ✅ / 8 |
| `Series[].DisplayName` | `单位m/s` |

---

## 5. 排错

| 现象 | 原因 |
| --- | --- |
| Palette 里没有 NXCharts | 插件没启用，或编辑器没重启 |
| 完全空白 | `SeriesKey` 为空（静默跳过，不报错）；或没灌数据 |
| 控件很小 / 被裁 | 父容器没给空间，默认 480×320 |
| 图例不出现 | `bShowLegend` 关了，或所有系列 `bShowLegend` = false |
| 曲线全是直线 | `Style.Curve` 还是 `Linear` |
| 没有填充 | `Style.bFillArea` 没开 |
| 数据更新了但画面不动 | 确认改的是**同一个 Widget 实例**（不是新建的） |
| 重开界面后运行时加的系列没了 | 正常行为：运行时系列活在实例上，重建后回到资产的初始值。需要持久化就自己存一份配置 |
| 细节面板看不到新字段 | 编辑器没重启 / 插件没重新编译 |

---

## 6. 还没实现的（别踩空）

| 能力 | 现状 |
| --- | --- |
| 柱状图 / 饼图 | `Kind` 字段在，但**所有 Kind 都按折线画**。散点可以用 `Line` + `bShowMarkers` 近似 |
| 图例换行 | 系列多时图例是单行/单列，会画到控件外 |
| 数据源对象（拉模型） | 未实现，目前只有推模型 API |
| 十字准线横线 | 只有竖线 |
| Y 轴标题竖排 | 横排放在绘图区上方 |

---

## 7. 数据从后端来怎么办

Widget API **必须在游戏线程调用**。网络回调如果在别的线程：

1. 在回调里把数据丢进一个队列（蓝图里用 Event Dispatcher + `Async` 节点，或在 C++ 里 `AsyncTask(ENamedThreads::GameThread, ...)`）；
2. 游戏线程每帧/Timer 里统一刷 `SetSeriesPoints`。

不要在每个网络包里直接调 `AddSeriesPoint`——高频时每次都会触发重建判定。
