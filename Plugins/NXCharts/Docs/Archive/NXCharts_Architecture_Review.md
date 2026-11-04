# NXCharts 架构评审（对照《UE5.8 原生 Chart 插件：技术分析与实施方案》）

> 评审对象：`Plugins/NXCharts`（单模块 Runtime 插件，`Binaries/Win64/UnrealEditor-NXCharts.dll` 已产出）
> 对照文档：桌面上的技术分析与实施方案
> 评审日期：2026-10-29

---

## 1. 总体结论

**方向是对的，但完成度停在了"能画一条线"，而不是"图表系统"。**

NXCharts 已经做出了方案 B 里最容易被忽略的那半边（Slate 自绘 + UMG 包装 + 绘制上下文收敛 +
组件化扩展点），缺的恰好是文档里工作量最大的那半边：**数据模型层**和**布局/文字层**。

用文档自己的验收标准衡量：

| 文档要求（阶段 1–3 验收） | 当前状态 |
| --- | --- |
| 插件编译、蓝图能创建控件 | ✅ 已达成（DLL 已产出） |
| 能绘制带坐标轴的空白图表 | ⚠️ 未达成：`UAxisComponent::CalculateAxisPoints` 返回空数组，无刻度、无标签、无网格 |
| 折线图正常显示 | ⚠️ 半达成：能画线，但无裁剪、无缓存、无多系列验证 |
| 柱状图、散点图 | ❌ 未开始 |
| 数据模型与图表类型分离 | ❌ 未做：数据直接住在渲染组件里 |

按文档的"初版可用"标准，大概完成了 30%，而且缺的 70% 里有 50% 集中在同一处：
**没有"数据 → 域 → 刻度 → 布局 → 几何 → 绘制"这条流水线**，现在是从"组件里的点"直接跳到"Slate 画线"。

---

## 2. 六个子系统的对标

| 子系统 | 文档要求 | NXCharts 现状 | 判定 |
| --- | --- | --- | --- |
| 数据系统 | 分类数据 + 数值 XY + 时间序列 | 无独立数据层，`TArray<FVector2D> DataPoints` 在系列组件内部 | ❌ 缺失 |
| 图表模型 | `FChartModel` / `FChartSeries` / 样式 | `UChartWidget` + `Modules[]` + `Components[]`，没有数据模型对象 | ⚠️ 形态不同，等价物不全 |
| 坐标系统 | 统一域、映射、刻度、标签 | `UCartesianCoordModule` 只有固定 padding + 两个范围字段；无刻度生成、无逆映射 | ⚠️ 骨架 |
| 布局系统 | 标题/图例/轴/绘图区分区计算 | 完全是硬编码常量 `Left=50, Top=30, Right=30, Bottom=50` | ❌ 缺失 |
| 渲染系统 | 网格/轴/线/柱/面/点 | `ChartDrawContext` 只有 `DrawLine` / `DrawRect`，**不能画文字** | ⚠️ 骨架 |
| 交互系统 | 悬停/提示/缩放/平移/选择 | 有命中测试分派循环；点击事件被注释；无 tooltip、无缩放 | ⚠️ 骨架 |

---

## 3. 值得保留的四个决定

这几处比文档的建议更贴合 UE 实际，不要因为对照文档而推翻：

### 3.1 Slate 自绘 + UMG 包装（`SLeafWidget` + `UWidget`）

正是文档 §2.2"形式二"。`SChartCanvas` 继承 `SLeafWidget`、`UChartWidget` 只做包装，
比文档里讨论的 `UUserWidget::NativePaint` 更适合"长期维护的插件"，理由和文档一致（Slate 控件能
集中处理 Layout/Paint/Input，未来还能进编辑器面板）。**保留。**

### 3.2 `ChartDrawContext` 作为绘制收敛层

把 `FSlateWindowElementList` + `FGeometry` + Layer 封装成一个纯 C++ 上下文，让所有组件只面对
`DrawLine/DrawRect`。这是文档 §3.2"把 Slate 细节收敛到 Core 层"的具体落地，方向完全正确。
**保留，但必须补图元**（见 §5.2）。

### 3.3 Module / Component 组件化，而不是单一 `FChartModel`

文档建议一个 `FChartModel` 装下全部。NXCharts 用 `UChartModule`（提供空间/上下文）+
`UChartComponent`（提供可绘制元素、可命中、可广播事件）的组合，配合
`Instanced` + `EditInlineNew` + `Blueprintable`，换来两个实打实的好处：

- UMG Designer 里可以直接配图表（Details 面板里加模块、加系列、改颜色）。
- 用户能在 **蓝图里子类化一个系列组件**，不用碰 C++ 就能定义新图表——这是纯 `FChartModel` 做不到的。

文档 §7 第三层说的"树图、桑基图保留自己的布局与绘制器"，用组件化表达比用枚举 + switch 干净。
**保留这个外壳，但数据和布局必须从组件里抽出来**（见 §4.1、§4.2）。

### 3.4 `UChartTheme : UDataAsset`

主题做成 DataAsset 便于项目级换肤，思路对。**但目前完全没被引用**（`grep` 全仓库只有声明和空 cpp），
属于"写在纸上但没接线"的配置。

---

## 4. 五个架构级问题（按修复成本排序）

### 4.1 【最贵】数据住在渲染器里，没有数据模型

`UlineSeriesComponent` 既是数据容器，又是坐标映射器，又是绘制器，又是命中测试器。

这不是"少写一个类"的问题，它直接导致：

1. **换图型要重新灌数据**——同一份数据想看折线还是柱状，现在无法复用。
2. **自动范围没法通用**——`UCartesianCoordModule::NotifyComponentDataChanged()` 是空实现，
   因为它没有任何办法知道"谁绑在我身上、你们的数据范围是多少"。文档 §9 建议的
   `virtual bool GetDataRange(...)` 就是这个缺口，现在只能靠在每个系列里各写一遍。
3. **序列化语义混乱**——Widget BP 序列化的是"数据 + 表现"，业务层（数字孪生的数据源）没法
   单独持有数据再一次性喂给多张图。
4. **实时追加无路可走**——`TArray<FVector2D>` 做 UPROPERTY，每秒追加 + 整体重排 + 全量重绘，
   文档 §9.2 明确点名的"全量重建"问题现在全中。
5. **无法做数据级操作**——排序、去重、聚合、抽稀、时间窗口裁剪，都得在组件里做。

**建议**：引入 `FChartSeriesData`（名称/类型/颜色/点集/版本号）+ `FChartDomain`（域），
由 `UChartWidget` 或一个 `UChartDataSet` 持有；`UChartComponent` 降级为"渲染器 + 配置"，
通过 `GetSeriesData()` 反查。这一步现在改最便宜（Content 目录是空的，没有任何 BP 资产要迁移）。

### 4.2 【最硬】没有布局阶段，也没有文字

`UCartesianCoordModule::GetPaddedPlotBounds` 返回常量 padding。这带来三个连锁问题：

1. **轴标签放不下**：真实图表的左边距必须由 Y 轴最宽标签实测宽度决定（`1200000` 和 `1` 差几个数量级）。
   固定 50px 在高 DPI、大数值、负数下必然溢出或留下大片空白。
2. **`ChartDrawContext` 不能画文字**——`FSlateDrawElement::MakeText` 一次都没调用。
   没有文字的图表不是图表：轴标签、图例、标题、tooltip 全靠它。
3. **文字测量需要一次"预布局"**：`FSlateFontMeasure`（`FSlateApplicationBase::Get().GetRenderer()->GetFontMeasureService()`）
   必须在画之前跑一遍，才能知道每根轴要预留多少像素。这是 Slate 自绘控件里最容易踩坑的地方，
   而且 `SLeafWidget` 没有子控件，Slate 不会替你算。

**建议**：加一个显式的 `FChartLayout` 阶段（每帧一次，尺寸/域/字体变化时才重算），
产出 `PlotRect` + 各轴刻度 + 标签尺寸。文档 §10 阶段 2 的"标题/轴/刻度/网格/标签/绘图区"
本质就是这一个类。

### 4.3 坐标转换不是唯一事实源

现在正变换在 `UlineSeriesComponent::MapDataToPixel`，而**逆变换根本不存在**。
文档 §6.3 警告的"图形位置与交互位置不一致"很快会依次出现在：tooltip 的 X 值反查、十字准线、
滚轮缩放锚点、框选区间、双 Y 轴。

同时 `MapDataToPixel` 有三个即时成本：

- 每次 `OnPaint` 调用一次，每次 `HitTest` 再调用一次（鼠标移动时每帧两次）。
- 每次返回一个新建的 `TArray<FVector2D>`，然后在 `DrawLine` 里再转成 `TArray<FVector2f>`——**每帧两次分配 + 两次拷贝**。
- `GetParentModule()` 是 O(N) 字符串比较，每次映射都跑一遍。

`ChartUtils::MapValue` 还吃 `float`：UE5 的 `FVector2D` 是 double，数据是 double，到映射这一层被砍成 float
（时间戳/大数值场景会丢精度——文档 §8.1 提过这个风险）。`FChartAxisRange` 也用 `float`，不一致。

**建议**：把 `FChartCartesianSpace`（域 + 绘图区 + `DataToLocal` / `LocalToData`）做成一等公民，
组件只持有它，几何结果缓存成 `TArray<FVector2f>`。

### 4.4 立即模式无缓存、无裁剪

`SChartCanvas::OnPaint` 每次都：遍历模块 → 遍历组件 → 各自重建坐标数组 → 重新 DoPaint。
没有区分文档 §9 要求区分的四级更新（数据 / 模型 / 布局 / 绘制）。

两个具体后果：

- **超界不裁剪**：`MapValue` 不 clamp，也没有 push clip。缩放/平移后曲线会直接画到轴标签和控件外。
  建议 `OutDrawElements.PushClip(FSlateClippingZone(...))` / `PopClip()` 包住绘图区。
- **点数直接等于每帧成本**：`MakeLines` 会把点数组放进绘制元素，100k 点 = 每帧 100k 点的拷贝 + 三角化 + AA。
  文档要求的"大数据量"档位（10k / 100k）现在没有一档能过。

**建议**：几何构建与绘制两段式（见 §6），几何按 `(数据版本, 域, PlotRect)` 缓存；
按像素列做 min/max 抽稀后，成本从 O(点数) 变成 O(图表宽度)——**抽稀不是优化项，是 100k 档位的前置条件**。

### 4.5 UObject 承担热路径 + 字符串绑定

- 组件用 `ParentModuleName`（`FName`）绑定模块，`GetParentModule()` 每次线性查找。
  在设计器里改了 `ModuleName` 或调换了数组顺序，绑定会**静默失效**，且没有任何报错。
- `Instanced` 数组在 Details 面板里很好用，但 `UChartWidget` 没有 `SynchronizeProperties` /
  `PostEditChangeProperty`，改完属性不刷新、Designer 里看不到变化。
- 热路径用 UObject 本身不是问题（配置对象低变更），问题在于**数据数组和每帧几何也挂在了 UObject 上**。

**建议**：UObject 只承载"配置与生命周期"，几何和域用纯 C++ 结构体缓存在 `SChartCanvas`/`UChartWidget` 上；
绑定关系改成"添加组件时直接传模块指针"+ 一份可校验的缓存，并在编辑器里对未绑定的组件打 warning。

---

## 5. 代码级问题清单

### 5.1 功能缺陷（会直接导致"看起来没效果"）

| 位置 | 问题 |
| --- | --- |
| `ChartWidget.cpp` `GetComponentByName` | 循环体没比较 `ComponentName`，直接 `break` 返回第一个组件 |
| `ChartCanvas.cpp` `OnMouseButtonDown` | 广播语句被注释，`OnChartItemClicked` 永远不会触发（蓝图收到空事件） |
| `AxisComponent.cpp` `CalculateAxisPoints` | 构造了 `Point` 却没 `Add`，最后 `return TArray<FVector2D>()`——轴永远画不出来；只有 X 分支 |
| `AxisComponent.h` `AxisThickness` | 无默认值，UObject 零初始化 → 线宽 0 |
| `AxisComponent.cpp` `HitTest` | 只调 `Super`（恒返回 -1），轴参与命中却永不被命中 |
| `CartesianCoordModule.cpp` | 画不透明白底，无视 `GlobalOption.BackgroundColor`；`GridColor` 声明了没用 |
| 全局 | `FChartGlobalOption` 的 `Padding` / `bAntiAlias` / `DefaultFontName` / `BackgroundColor` **一个都没被读取**；抗锯齿是组件里写死的 `true` |
| 全局 | `UChartTheme` 完全未被引用 |

### 5.2 结构性缺陷

| 位置 | 问题 |
| --- | --- |
| `ChartDrawContext.h` | 只有 `DrawLine`/`DrawRect`；缺 `DrawText` / `DrawCircle` / `DrawPolygon` / `DrawDashedLine` / 裁剪入栈出栈 |
| `ChartDrawContext.h` | 用 `FCoreStyle` 但没 include `Styling/CoreStyle.h`（当前靠传递包含，脆弱） |
| `ChartDrawContext.h` | 每个图元都 `ReserveLayer()` 递增，z 序是隐式的；建议语义分层（背景/网格/轴/系列/覆盖） |
| `ChartCanvas.cpp` | `OnPaint` 直接用 `bParentEnabled` / `InWidgetStyle` 都没有，控件禁用时不会变灰 |
| `ChartCanvas.cpp` | 固定 `ComputeDesiredSize(400,300)`，与 UMG 期望尺寸/`UWidget::GetPaletteCategory` 脱节 |
| `ChartCanvas.cpp` | `OnMouseMove` 恒 `Handled()`，且无光标反馈 |
| `CartesianCoordModule.h` | `CachedXRange.IsZero()` 当"未设置"哨兵，语义脆弱（应为 `bHasRange`） |
| `Build.cs` | 公开头文件 `ChartDrawContext.h`、`ChartTheme.h` 泄漏了 `SlateCore` / `Engine` 的头，但这两个模块在 `PrivateDependency` 里——按 UE 规则应提到 `PublicDependency` |
| `Config/DefaultNXCharts.ini` | 三条 CoreRedirect 全是无效的：`FChartGlobalOption → ChartGlobalOption`（新名不存在）、`MyClass → UlineSeriesComponent`、以及自指重定向；脚本名不带 `U` 前缀，应为 `LineSeriesComponent`。建议清空 |
| `LineSeriesComponent` | 类名 `UlineSeriesComponent`（小写 l），蓝图面板里会显示成 `Uline Series Component`。**现在 Content 为空，改名零成本**，以后就得写 CoreRedirect |

---

## 6. 对实施方案文档的三点保留意见

### 6.1 引擎版本前提要改

文档按 **UE5.8** 写，但当前 [5.7 才是已发布版本](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-7-release-notes?application_version=5.7)，
你的工程 `MyProject2.uproject` 的 `EngineAssociation` 也是 `5.7`。
对本插件其实没影响（这轮用到的 Slate/UMG API 在 5.5–5.7 之间没有破坏性变化），
但文档里那三条 `?application_version=5.8` 的锚点链接需要换成 5.7，否则查资料会绕路。

### 6.2 `IChartRenderer` 纯虚接口不是该抽象的东西

文档 §3.1 给了 `IChartRenderer { virtual void Render(const FChartRenderContext&) }`。
建议**不要**这么做：

1. 纯 C++ 虚接口会把蓝图用户挡在外面，而你已经有 `UChartComponent::CollectDrawElements` 这个
   BP 可子类化的等价物——它比 `IChartRenderer` 更好。
2. 你真正需要抽的是**几何构建**，不是渲染。Slate 本身就是后端，文档 §3.2 自己也承认"GPU 不一定更快"。
3. 抽象成两段式后，收益是全套的：

```
BuildGeometry(Space, Data, Out)   // 纯 C++：可缓存、可单测、可与后端无关
Paint(Context, Geometry)          // 只做 Slate 调用
```

换后端时只重写 `Paint`；做性能优化时只改 `BuildGeometry` 的缓存策略；写单元测试时完全不碰 Slate。

### 6.3 P0 的图表类型排错了

文档把"折线 / 柱状 / 散点"并列 P0。实际上：

- **散点图 ≈ 折线图 + 点标记 + 点命中**，几乎不增加任何架构信息量（可以先塞进折线组件里）。
- **柱状图才是会倒逼抽象的那个**：它引入"分类轴 / band 尺度 / 带宽 / 组内偏移 / 堆叠"，
  会迫使你把"域"抽象成 `FChartScale`（Linear / Band / Time）而不是两个 `FVector2D`。

所以验证顺序建议：**折线（含散点标记）→ 柱状 → 其他**。柱状要早做，别拖到 M3 之后。

---

## 7. 目标架构（在现有代码上长，不推倒重来）

```
UChartWidget (UWidget)                      ← 保留，补 SynchronizeProperties / GetPaletteCategory
  ├─ FChartGlobalOption / UChartTheme        ← 保留，接进绘制（现在没接线）
  ├─ TArray<UChartSeriesData>  (新)          ← 数据层，与渲染分离
  └─ SChartCanvas (SLeafWidget)              ← 保留
        │
        ├─ FChartLayout (新, 纯C++)           ← 每帧一次：域→刻度→文字实测→PlotRect
        │     └─ FChartCartesianSpace (新)    ← 唯一坐标事实源：DataToLocal / LocalToData
        │
        ├─ UChartModule[]                     ← 保留：提供空间/上下文（Cartesian/Legend/Zoom）
        └─ UChartComponent[]                  ← 保留：渲染器 + 配置
              ├─ UAxisComponent               ← 重写（刻度/标签/网格）
              ├─ ULineSeriesComponent         ← 改名 ULineSeriesComponent + 几何缓存 + 裁剪
              └─ UBarSeriesComponent          ← 新增（验证抽象）
                    │
                    └─ BuildGeometry() → FChartPaintContext::DrawLine/DrawText/...
```

类映射表：

| 现有 | 处置 |
| --- | --- |
| `UChartWidget` | 保留；加 `SynchronizeProperties`、`GetPaletteCategory`、`SetGlobalOption`、系列便捷 API |
| `SChartCanvas` | 保留；`OnPaint` 改为读"已编译的帧快照"，不再直接遍历 UObject |
| `ChartDrawContext` | 保留并扩充为 `FChartPaintContext`（文字/圆/多边形/虚线/裁剪） |
| `UChartModule` | 保留；职责收敛为"提供空间与上下文" |
| `UCartesianCoordModule` | 拆分：范围/刻度逻辑抽到 `FChartDomain` + `FChartTickGenerator`，自身只留 Layout 与 ClipRect |
| `UChartComponent` | 保留；拆成"数据/配置" + `BuildGeometry` + `Paint` + `HitTest` |
| `UAxisComponent` | 重写 |
| `UlineSeriesComponent` | 保留改造（改名、解耦数据、缓存几何） |
| `ChartUtils` | 扩充：nice numbers 刻度、数值格式化、clamp、抽稀 |
| `UChartTheme` | 保留，接入绘制 |

---

## 8. 落地计划（替换文档的六阶段，更聚焦）

### M0 修复阻塞项（0.5 天）

1. `GetComponentByName` 按名匹配；`OnMouseButtonDown` 恢复广播；`AxisThickness` 默认 1.0。
2. 清空无效的 `DefaultNXCharts.ini` CoreRedirects。
3. `Build.cs`：`Slate` / `SlateCore` / `Engine` 提到 `PublicDependency`。
4. `UlineSeriesComponent` → `ULineSeriesComponent`（趁 Content 空的最后窗口）。
5. `ChartDrawContext.h` 补 `Styling/CoreStyle.h`。

**验收**：编译通过；蓝图能点到 `OnChartItemClicked`。

### M1 数据 + 域 + 布局 + 文字（1.5–2 周）★ 分水岭

1. `FChartSeriesData` / `FChartDomain` / `FChartScale`。
2. `ChartTicks::MakeLinearTicks`（1-2-5 × 10ⁿ）+ 数值格式化。
3. `FChartLayout`：用 `FSlateFontMeasure` 实测轴标签宽度 → 推 `PlotRect`。
4. `FChartPaintContext::DrawText` + 裁剪入栈。
5. 重写 `UAxisComponent`：主轴 + 刻度 + 标签 + 网格。
6. `UCartesianCoordModule` 接 `GlobalOption.Padding` 与 `BackgroundColor`。

**验收**：一张带标题、双轴、刻度标签、网格线的空图表；改窗口尺寸时布局自适应；`GlobalOption` 真的生效。

### M2 折线跑通 + 交互底座（1 周）

1. `FChartCartesianSpace` 统一正/逆映射，删掉 `MapDataToPixel`。
2. 折线几何缓存 + 裁剪 + 点标记。
3. 十字准线 + tooltip（用逆向映射反查数据）。
4. 抽稀（min/max per pixel column）作为必选项。

**验收**：3 条系列 / 10 万点 / 拖动窗口 60fps；tooltip 数值与图形位置一致。

### M3 柱状图验证抽象（3–4 天）

引入 `FChartBandScale` + 分类轴，做单系列 → 分组 → 堆叠。这一步如果 `FChartScale` 抽象没做对，
会在这里被狠狠打回一次——所以要早做。

### M4 交互与性能（1–2 周）

缩放/平移（改域不改数据）、图例模块（点击隐藏系列）、实时追加（环形缓冲 + 只重建尾部）、
文档 §10 的五档性能测试表、打包验证。

---

## 9. 一句话总结

现有代码的**骨架选择是对的**（Slate 自绘 + 绘制上下文 + 组件化扩展点），
但它现在是一条"组件直接画像素"的捷径；文档的价值在于把中间那三层（数据 / 域 / 布局）补回来。
**不要为了对齐文档推倒组件化设计，也不要为了保住现有代码跳过数据层**——
两者是可以合并的，而且合并的唯一时机就是现在（Content 是空的，没有任何资产包袱）。
