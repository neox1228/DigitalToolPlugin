# NXCharts 使用文档

> 面向：要在 UMG 里**用**这个图表的人
> 其他文档：`NXCharts_Manual.md`（架构/技术栈）· `NXCharts_API.md`（接口参考）

---

## 0. 开始之前

1. 重新编译项目（改了 C++ 就必须编译）
2. 重启编辑器（蓝图节点列表只有在重启后才会刷新）
3. 如果之前配过图表：**Spec 的字段路径刚变过**（见文末「已知影响」），需要重配一次

---

## 1. 五分钟做一个折线图

### ① 放控件

Widget Blueprint → Palette 搜 **NXChartWidget**（分类 `NXCharts`）→ 拖进画布 → 拉大到 600×360。

### ② 灌数据

在 `Event Construct` 里：

```
AddSeries(
    SeriesKey   = "cpu"                       ← 必填，运行时唯一句柄
    Kind        = Line
    Color       = 青色
    Points      = [ (0,120), (1,180), (2,150), (3,240) ]
    DisplayName = "单位m/s"                    ← 图例和提示框显示这个名字
)
```

`Points` 的 X 是横轴值（时间、序号都行），Y 是纵轴值。

### ③ 看效果

Play → 图表立即出现。鼠标移进去有十字准星和数值提示。

---

## 2. 数据怎么更新

| 场景 | 节点 |
| --- | --- |
| 实时追加一个点 | `AddSeriesPoint("cpu", (4, 200))` |
| 整批替换 | `SetSeriesPoints("cpu", 新数组)` |
| 只保留最近一段 | `RemoveSeriesPointsInXRange("cpu", 0, 50)` |
| 清空但保留样式 | `ClearSeries("cpu")` |
| 删掉整条线 | `RemoveSeries("cpu")` |

**建议**：实时曲线用 `AddSeriesPoint` + 定期 `RemoveSeriesPointsInXRange` 裁历史窗口，内存不会无限涨。

性能参考：3000 点以内随便刷；超过 4000 点会自动做像素级抽稀，视觉不变但几何计算量下降。

---

## 3. 外观怎么调（Details 面板）

选中控件 → Details → 展开 **`Spec`**。每个块直接挂在 Spec 下，一个块对应一个绘制组件：

| 块 | 里面有什么 |
| --- | --- |
| `Title` | 标题文字 `Text`、字体 `Font`（留空=不画标题） |
| `Background` | 背景色 `Color`、是否渐变 `bGradient`、渐变底 `ColorBottom` |
| `Grid` | 网格色 `Color`、是否虚线 `bDashed`、虚线长度/间隔 |
| `Legend` | 是否显示 `bShow`、位置 `Position`（Top/Bottom/Right）、字体、文字色 |
| `Colors` | 跨组件共享色：轴线色 `AxisColor`、刻度色 `LabelColor`、标题色 `TitleColor` |
| `Annotations` | 注解数组（阈值线/色带），见第 5 节 |
| `AxisLabelFont` | 轴标签和提示框共用的字体 |

### 系列的样式

```
SetSeriesStyle("cpu", Style)       ← 整块替换
SetSeriesColor("cpu", 红色)         ← 只改颜色
SetSeriesCurve("cpu", Smooth)      ← 只改连线方式
SetSeriesDisplayName("cpu", "转速") ← 只改图例文案
SetSeriesVisible("cpu", false)     ← 显隐（图例会变暗，还能点回来）
```

`Style` 里能调的：

| 字段 | 效果 |
| --- | --- |
| `LineThickness` | 线宽 |
| `bShowMarkers` / `MarkerSize` | 数据点圆点 |
| `Curve` | `Linear` / `Smooth`（单调三次，不过冲）/ `Step` |
| `bFillArea` / `AreaFillOpacity` | 曲线下方渐变填充 |
| `bGlow` / `GlowThickness` | 霓虹发光线 |

---

## 4. 轴怎么配

用节点 `SetAxisSpec(EChartAxis, FChartAxisSpec)` 改某一根轴。

| 字段 | 说明 |
| --- | --- |
| `AxisTitle` / `Unit` | 轴标题；`Unit` 非空时拼成「标题 (单位)」 |
| `TickSuffix` | 每个刻度后缀，填 `s` → `1s / 2s / 3s` |
| `Format` | `Auto` / `Integer` / `OneDecimal` / `TwoDecimals` / `Percent` / `Scientific` |
| `DesiredTickCount` | 期望刻度数（实际由 1-2-5 步长决定，不会精确等于） |
| `bShowGrid` / `bShowLabels` | 网格 / 刻度标签开关 |
| `DomainMode` | `Auto` 跟数据 / `Manual` 用 `ManualDomain` |
| `ManualDomain` | 手动范围（`DomainMode=Manual` 时才生效） |
| `bUseHardLimits` | **强烈建议开启**，防止缩放把轴撑成负数 |
| `HardLimits` | `X` = 下界，`Y` = 上界。数据非负就填 `(0, 上限)` |

**最常用的一个配置**（防止负数、防止缩飞）：

```
bUseHardLimits = true
HardLimits     = (0, 1500)
```

---

## 5. 注解：阈值线 / 分界线 / 色带

Details → `Spec` → **`Annotations`** → `+` 加一条。

| `Kind` | 效果 | 用哪个字段定位 |
| --- | --- | --- |
| `HorizontalLine` | 水平线（阈值线） | `Value` = Y 值 |
| `VerticalLine` | 垂直线（时间游标） | `Value` = X 值 |
| `Band` | 水平色带（告警区间） | `Value` ~ `RangeEnd` |

其余字段：`Color`、`Thickness`、`bDashed`、`Label`（线尾文字，留空不画）。

画在网格之上、曲线之下，不挡数据。超出当前视野的注解会自动不画。

---

## 6. 交互

### 内置的（不用写代码）

| 操作 | 效果 |
| --- | --- |
| 鼠标移入 | 竖线 + 各系列最近点 + 数值提示框 |
| 滚轮 | 以光标为锚点缩放（受 `HardLimits` 约束） |
| **中键拖拽** | 平移 |
| 左键点图例 | 切换该系列显隐 |
| **双击绘图区** | 复位到自动范围 |

⚠️ 平移是**中键**，不是左键。

### 自己做一个复位按钮

```
点击按钮时：
  Chart.ClearDomainOverride()        ← 复位
```

想让按钮只在"缩放/平移过"时出现：

```
Chart.IsViewModified()  → true/false   ← 每 Tick 或事件里查
```

### 外部十字准星（多图联动）

```
Chart.SetCrosshairX(1.5)      ← 传数据坐标的 X，吸附最近真实点
Chart.ClearCrosshair()        ← 关闭
Chart.IsCrosshairActive()     ← 查询
```

- 激活期间鼠标悬停让位，不会出现两条竖线
- 信息框固定在绘图区顶部，跟随竖线横移
- 同一个 X 传给多个图表 = 多图联动（ECharts 的 axisPointer.link 效果）

想知道吸附到了哪条系列的哪个点：

```
Event OnCrosshairSnapped(SeriesKey, PointIndex, DataValue)   ← 每条系列各触发一次
```

---

## 7. 增删改查速查

### 系列

| 操作 | 节点 |
| --- | --- |
| 增 | `AddSeries` / `AddOrUpdateSeries` |
| 查 | `HasSeries` / `GetSeriesSpec` / `GetSeriesKeys` |
| 改 | `SetSeriesStyle` / `SetSeriesColor` / `SetSeriesCurve` / `SetSeriesDisplayName` / `SetSeriesVisible` |
| 删 | `RemoveSeries` |

### 点

| 操作 | 节点 |
| --- | --- |
| 增 | `AddSeriesPoint`（追加）/ `InsertSeriesPoint`（按索引插入） |
| 查 | `GetSeriesPoints` / `GetSeriesPointCount` / `GetSeriesPointAt` |
| 改 | `SetSeriesPoints`（整体）/ `SetSeriesPointAt`（单点） |
| 删 | `ClearSeries` / `RemoveSeriesPointAt` / `RemoveSeriesPointsInXRange` |

---

## 8. 常见坑

**① 节点搜不到**
→ 没编译或没重启编辑器。

**② 加了系列但图上没线**
→ 检查 `SeriesKey` 是不是空的（空 Key 会被静默忽略）；再检查 `Points` 是否为空。

**③ 提示框里名字是英文 Key**
→ 没设 `DisplayName`，会回退显示 `SeriesKey`。

**④ 缩放后轴出现负数**
→ 没开 `bUseHardLimits`。自动域本身会上下加 5% 留白，数据最小是 0 时就会变负。

**⑤ 缩放放不大了**
→ 顶到 `HardLimits` 边界了，这是预期行为。

**⑥ 曲线画出来是折线，设了 Smooth 也没用**
→ `Smooth` 在「点数 < 3」或「X 不是递增」时会自动退化成折线（避免画出乱线）。

**⑦ 拖不动图**
→ 平移是**中键**拖拽。

**⑧ 多图联动时竖线位置不一致**
→ 检查各图的 X 轴范围是否相同；不同的域会把同一个数据值映射到不同像素位置。

---

## 9. 已知影响（本次架构调整）

Spec 的字段路径重构过，旧的 Widget Blueprint 里保存的配置**不会自动迁移**：

| 旧 | 新 |
| --- | --- |
| `Options.Title`（文字） | `Title.Text` |
| `Options.TitleFont` | `Title.Font` |
| `Options.bShowLegend` | `Legend.bShow` |
| `Options.LegendPosition` | `Legend.Position` |
| `Options.LegendFont` | `Legend.Font` |
| `Options.LegendTextColor` | `Legend.TextColor` |
| `Options.BackgroundColor` | `Background.Color` |
| `Options.bGradientBackground` | `Background.bGradient` |
| `Options.BackgroundColorBottom` | `Background.ColorBottom` |
| `Options.GridColor` | `Grid.Color` |
| `Options.bDashedGrid` | `Grid.bDashed` |
| `Options.GridDashLength` / `GapLength` | `Grid.DashLength` / `GapLength` |
| `Options.AxisColor` / `LabelColor` / `TitleColor` | `Colors.*` |
| `Options.Padding` | `Padding` |
| `Options.AxisLabelFont` | `AxisLabelFont` |
| `Options.bAntialias` | `bAntialias` |
**默认值已经还原成之前的配色**，所以视觉上差别不大；但如果你手动调过颜色，需要重新配一次。

---

## 10. v0.3 变化（2026-11）

### 10.1 配置从 Details 搬到 JSON 文件

以前是在 Details 里逐字段展开 `Spec`；现在只填一个路径：

```
NXChartWidget → NXCharts → SpecPath = "NXCharts/Demo.json"
```

路径**相对 Content 目录**。留空使用内置默认（空白图）。

### 10.2 JSON 怎么写

```json
{
  "title":      { "text": "CPU 使用率", "fontSize": 14 },
  "background": { "color": "#05080D", "gradient": true, "colorBottom": "#02040A" },
  "grid":       { "color": "#282F3D", "dashed": false },
  "legend":     { "show": true, "position": "bottom", "textColor": "#B8C4D6" },
  "colors":     { "axis": "#596680", "label": "#B8C4D6", "title": "#EAF2FF" },
  "axisX": { "title": "时间", "unit": "s", "tickSuffix": "s", "desiredTicks": 6 },
  "axisY": { "title": "占用", "unit": "%", "format": "integer", "hardLimits": true, "min": 0, "max": 100 },
  "series": [
    { "key": "cpu", "name": "CPU", "color": "#3ED6C6", "type": "line", "curve": "smooth", "fill": true, "glow": true }
  ],
  "annotations": [
    { "kind": "horizontalLine", "value": 80, "color": "#FF6B4A", "label": "警戒线", "dashed": true }
  ]
}
```

字段全部可选，颜色用 `#RRGGBB` / `#RRGGBBAA`。**数据点不写进 JSON**，仍用 `AddSeries` / `SetSeriesPoints` 喂。

### 10.3 改完自动生效

保存 JSON 后 **0.5 秒内自动重载**，不用重启、不用点按钮。只重载配置不动数据，曲线不会闪断。

解析失败时保留上一份配置，并在输出日志里给一条 Warning。

### 10.4 常见问题

| 现象 | 原因 |
| --- | --- |
| 图表空白 | `SpecPath` 为空或路径不在 Content 下 |
| 改了 JSON 没反应 | 路径写错，或文件不在 Content 目录（看日志 Warning） |
| 颜色不生效 | 十六进制格式不对，必须 `#RRGGBB` |
| 曲线没出来 | 系列只定义了样式，还需要 `AddSeries` / `SetSeriesPoints` 喂数据 |