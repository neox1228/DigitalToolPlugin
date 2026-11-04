# NXCharts V1 / V2 目录与架构对照

> V1 = 当前仓库里的实际代码（24 个文件，实测 870 行）
> V2 = `NXCharts_Architecture_V2.md` 描述的目标结构（阶段 A 落地）
> 日期：2026-10-30

---

## 0. 一句话对照

| | V1 | V2 |
| --- | --- | --- |
| 目录按什么切 | **角色**（模块 / 组件） | **阶段**（配置 → 数据 → 布局 → 帧 → 图元） |
| 谁是权威状态 | UObject 数组（序列化进 BP） | `FChartSpec`（值）+ `FChartSeriesBuffer` |
| 依赖方向 | Widget ↔ Component 双向 | Widget → Spec/Data → Frame → Render 单向 |
| 运行时改系列数 | 需要重新配置 UMG 并打包 | `AddOrUpdateSeries` / `RemoveSeries` |
| 轴/网格/图例 | 用户装配的对象 | Frame 根据 AxisSpec 生成 |

---

## 1. V1：真实目录树（含处置标注）

```
Plugins/NXCharts/
├─ NXCharts.uplugin                          23 行  [留]   版本号升 2
├─ Config/DefaultNXCharts.ini                 4 行  [改]   清空 3 条无效 CoreRedirect
├─ Resources/Icon128.png                             [留]
├─ Docs/                                             [留]   + 设计文档
├─ Binaries/ Intermediate/                           [忽略] 构建产物
└─ Source/NXCharts/
   ├─ NXCharts.Build.cs                      55 行  [改]   Slate/SlateCore/Engine 提 Public
   ├─ Public/
   │  ├─ NXCharts.h                          14 行  [留]
   │  ├─ Core/
   │  │  ├─ ChartCanvas.h                    38 行  [移]   → Private/Widget/
   │  │  ├─ ChartDrawContext.h               57 行  [改]   → Core/ChartPaintContext.h
   │  │  ├─ ChartGlobalOption.h              42 行  [删]   并入 Spec/ChartSpec.h
   │  │  ├─ ChartTheme.h                     43 行  [留]   Spec 里用 TSoftObjectPtr 引用
   │  │  └─ ChartUtils.h                     28 行  [改]   → Layout/ChartMath.h
   │  ├─ Feature/                                   [删]   整个目录消失
   │  │  ├─ Modules/
   │  │  │  ├─ ChartModule.h                 40 行  [删]   职责拆到 Space/Layout/Frame
   │  │  │  └─ CartesianCoordModule.h        30 行  [拆]   → Layout/ChartSpace.h + ChartLayout.h
   │  │  └─ Components/
   │  │     ├─ ChartComponent.h              56 行  [改]   → Render/ChartSeriesRenderer.h
   │  │     ├─ AxisComponent.h               60 行  [改]   → Render/AxisLayerRenderer.h
   │  │     └─ LineSeriesComponent.h         33 行  [改]   → Render/LineSeriesRenderer.h
   │  └─ Widget/
   │     └─ ChartWidget.h                    62 行  [改]   加入 Spec / 6 个运行时 API
   └─ Private/
      ├─ NXCharts.cpp                        12 行  [留]
      ├─ Core/
      │  ├─ ChartCanvas.cpp                 107 行  [移+改] OnPaint 只读 Frame
      │  ├─ ChartTheme.cpp                    4 行  [留]
      │  └─ ChartUtils.cpp                   18 行  [改]   → Layout/ChartMath.cpp
      ├─ Feature/
      │  ├─ Modules/
      │  │  ├─ ChartModule.cpp                4 行  [删]
      │  │  └─ CartesianCoordModule.cpp      35 行  [拆]   删掉画白底的部分
      │  └─ Components/
      │     ├─ ChartComponent.cpp            41 行  [重写] 去掉 Outer 回跳 / 名称查找
      │     ├─ AxisComponent.cpp             32 行  [重写] 当前实现不可用
      │     └─ LineSeriesComponent.cpp       48 行  [改]   几何缓存 + 裁剪
      └─ Widget/
         └─ ChartWidget.cpp                  74 行  [重写] 接 ApplySpec / Reconcile / 渲染器池
```

**V1 合计**：24 个文件（22 源文件 + Build.cs + uplugin），**870 行**。
其中真正干活的是 13 个「改造/拆解」文件，共 671 行；`Core/` + `Feature/` + `Widget/` 三层就是这个插件的全部。

处置统计：**保留 5 / 小改 1 / 移动 2 / 改造 13 / 删除 3**。

---

## 2. V2：目标目录树（含新增标注）

```
Plugins/NXCharts/
├─ NXCharts.uplugin                                  [留]
├─ Config/DefaultNXCharts.ini                        [改]
├─ Resources/Icon128.png                             [留]
├─ Docs/                                             [留]
└─ Source/NXCharts/
   ├─ NXCharts.Build.cs                              [改]
   ├─ Public/
   │  ├─ NXCharts.h                                  [留]
   │  ├─ Core/                                       ── 画布基础设施
   │  │  ├─ NXChartsTypes.h                          [新]  EChartSeriesKind / EChartAxis / FChartBarRect
   │  │  ├─ ChartPaintContext.h                      [改]  ← ChartDrawContext.h（+文字/圆/多边形/虚线/裁剪/分层）
   │  │  └─ ChartTheme.h                             [留]
   │  ├─ Spec/                                       ── 配置：唯一事实源，纯值
   │  │  ├─ ChartSpecTypes.h                         [新]  FChartSeriesStyle / FChartSeriesSpec / FChartAxisSpec
   │  │  └─ ChartSpec.h                              [新]  FChartSpec + FChartOptionsSpec + FChartDiagnostics
   │  ├─ Data/                                       ── 数据：高频独立通道
   │  │  ├─ ChartSeriesBuffer.h                      [新]  FChartSeriesBuffer（含环形缓冲 + Rev）
   │  │  └─ ChartDataSource.h                        [新]  UChartDataSource（BP 可扩展）
   │  ├─ Layout/                                      ── 数学与布局：编译的静态部分
   │  │  ├─ ChartMath.h                              [改]  ← ChartUtils.h
   │  │  ├─ ChartSpace.h                             [新]  FChartDomain / FChartScale / FChartCartesianSpace
   │  │  ├─ ChartTicks.h                             [新]  FChartTick + 1-2-5 刻度生成 + 格式化
   │  │  └─ ChartLayout.h                            [新]  FChartLayout（分区 + 文字实测 + plotRect）
   │  ├─ Frame/                                      ── 帧：脏标驱动（= V2 文档的"编译层"）
   │  │  ├─ ChartFrame.h                             [新]  FChartFrame / FChartSeriesGeometry（POD）
   │  │  └─ ChartReconciler.h                        [新]  EChartDirty / FChartPatch / ApplySpec 接口
   │  ├─ Render/                                     ── 图元：只读 Frame 绘制
   │  │  ├─ ChartSeriesRenderer.h                    [改]  ← ChartComponent.h（去数据/去回跳）
   │  │  ├─ ChartRendererRegistry.h                  [新]  Kind → UClass（含 BP 注册）
   │  │  ├─ LineSeriesRenderer.h                     [改]  ← LineSeriesComponent.h
   │  │  └─ AxisLayerRenderer.h                      [改]  ← AxisComponent.h（不再由用户装配）
   │  └─ Widget/
   │     └─ ChartWidget.h                             [改]  唯一对外入口
   └─ Private/
      ├─ NXCharts.cpp                                 [留]
      ├─ Core/
      │  ├─ ChartPaintContext.cpp                     [新]  图元多了，从 header-only 转出 .cpp
      │  └─ ChartTheme.cpp                            [留]
      ├─ Data/
      │  ├─ ChartSeriesBuffer.cpp                     [新]
      │  └─ ChartDataSource.cpp                       [新]
      ├─ Layout/
      │  ├─ ChartMath.cpp                             [改]
      │  ├─ ChartSpace.cpp                            [新]
      │  ├─ ChartTicks.cpp                            [新]
      │  └─ ChartLayout.cpp                           [新]
      ├─ Frame/
      │  ├─ ChartFrameBuilder.h                       [新]  域→刻度→布局→几何 的编排
      │  ├─ ChartFrameBuilder.cpp                     [新]
      │  └─ ChartReconciler.cpp                       [新]  diff → patch → 脏标（唯一值得单测的部分）
      ├─ Render/
      │  ├─ ChartSeriesRenderer.cpp                   [改]
      │  ├─ LineSeriesRenderer.cpp                    [改]
      │  └─ AxisLayerRenderer.cpp                     [改]
      └─ Widget/
         ├─ ChartWidget.cpp                           [改]
         ├─ ChartCanvas.h                             [移]  ← Public/Core/ChartCanvas.h
         └─ ChartCanvas.cpp                           [移+改]
```

**V2 阶段 A 合计**：约 38 个文件，**估算 3,000–3,800 行**（含头文件与注释）。
处置统计：**保留 5 / 小改 1 / 移动 2 / 改造 13 / 删除 3 / 新增 19**。

---

## 3. 逐文件映射表（24 → 38）

| # | V1 文件 | 行 | V2 去向 | 动作 |
| --- | --- | -- | --- | --- |
| 1 | `Public/NXCharts.h` | 14 | 同 | 保留 |
| 2 | `Private/NXCharts.cpp` | 12 | 同 | 保留 |
| 3 | `NXCharts.Build.cs` | 55 | 同 | 改：Slate/SlateCore/Engine → Public |
| 4 | `NXCharts.uplugin` | 23 | 同 | 保留，Version 升 2 |
| 5 | `Public/Core/ChartCanvas.h` | 38 | `Private/Widget/ChartCanvas.h` | 移动（转实现细节，头文件只留前置声明） |
| 6 | `Private/Core/ChartCanvas.cpp` | 107 | `Private/Widget/ChartCanvas.cpp` | 移动 + 改：OnPaint 只读 Frame |
| 7 | `Public/Core/ChartDrawContext.h` | 57 | `Core/ChartPaintContext.h` + `.cpp` | 改：扩 6 类图元 + 裁剪 + 语义分层 |
| 8 | `Public/Core/ChartGlobalOption.h` | 42 | `Spec/ChartSpec.h`（FChartOptionsSpec） | **删除**，并入并接线 |
| 9 | `Public/Core/ChartTheme.h` | 43 | 同 | 保留 |
| 10 | `Private/Core/ChartTheme.cpp` | 4 | 同 | 保留 |
| 11 | `Public/Core/ChartUtils.h` | 28 | `Layout/ChartMath.h` | 改：改名 + 扩展（clamp/抽稀） |
| 12 | `Private/Core/ChartUtils.cpp` | 18 | `Layout/ChartMath.cpp` | 改 |
| 13 | `Public/Feature/Modules/ChartModule.h` | 40 | — | **删除** |
| 14 | `Private/Feature/Modules/ChartModule.cpp` | 4 | — | **删除** |
| 15 | `Public/Feature/Modules/CartesianCoordModule.h` | 30 | `Layout/ChartSpace.h` | 拆：域/尺度/空间 |
| 16 | `Private/Feature/Modules/CartesianCoordModule.cpp` | 35 | `Layout/ChartLayout.cpp` | 拆：padding/plotRect，删掉画白底 |
| 17 | `Public/Feature/Components/ChartComponent.h` | 56 | `Render/ChartSeriesRenderer.h` | 降级：去数据、去 `ParentModuleName`、去 `GetChartWidget` |
| 18 | `Private/Feature/Components/ChartComponent.cpp` | 41 | `Render/ChartSeriesRenderer.cpp` | 重写：不再沿 Outer 回跳 |
| 19 | `Public/Feature/Components/AxisComponent.h` | 60 | `Render/AxisLayerRenderer.h` | 改：由 Frame 驱动，用户不再装配 |
| 20 | `Private/Feature/Components/AxisComponent.cpp` | 32 | `Render/AxisLayerRenderer.cpp` | 重写（当前实现不可用） |
| 21 | `Public/Feature/Components/LineSeriesComponent.h` | 33 | `Render/LineSeriesRenderer.h` | 改：改名 + 去数据 + 加 `RendererOverride` 入口 |
| 22 | `Private/Feature/Components/LineSeriesComponent.cpp` | 48 | `Render/LineSeriesRenderer.cpp` | 改：几何缓存 + 裁剪 + 抽稀 |
| 23 | `Public/Widget/ChartWidget.h` | 62 | 同 | 大改：`FChartSpec` + 6 个运行时 API + 渲染器池 |
| 24 | `Private/Widget/ChartWidget.cpp` | 74 | 同 | 重写：接 ApplySpec / Reconcile / 池化 |

新增 19 个：

| 分组 | 新增文件 | 职责 |
| --- | --- | --- |
| Core (2) | `NXChartsTypes.h`、`ChartPaintContext.cpp` | 公共枚举、图元实现 |
| Spec (2) | `ChartSpecTypes.h`、`ChartSpec.h` | 运行时配置的事实源 |
| Data (4) | `ChartSeriesBuffer.h/.cpp`、`ChartDataSource.h/.cpp` | 高频数据通道 |
| Layout (6) | `ChartSpace.h/.cpp`、`ChartTicks.h/.cpp`、`ChartLayout.h/.cpp` | 域 / 刻度 / 布局 |
| Frame (4) | `ChartFrame.h`、`ChartFrameBuilder.h/.cpp`、`ChartReconciler.cpp` | 帧与协调 |
| Render (1) | `ChartRendererRegistry.h` | Kind → UClass |

---

## 4. 目录哲学：横切 vs 纵切

**V1 —— 按角色横切**

```
Core/            基础设施（谁都能用）
Feature/Modules/       "服务"（坐标系）
Feature/Components/    "图元"（轴、折线）
Widget/          UMG 入口
```

问题：Module 和 Component 都站在"绘制"这一格，只是角色不同。所以「该做成 Module 还是 Component」
每次都要重新判断；轴要认识坐标系，只能靠 `ParentModuleName` 字符串互找。

**V2 —— 按阶段纵切**

```
Core/     画布基础设施
Spec/     配置（值，低频）
Data/     数据（高频）
Layout/   数学与布局（静态部分）
Frame/    帧与协调（脏标驱动）
Render/   图元（只读帧）
Widget/   UMG 入口
```

好处：目录顺序 = 流水线顺序 = 数据流方向。加一个图表类型只需在 `Render/` 加一个文件；
加一种坐标只需在 `Layout/` 加一个 Scale；加一个交互只需消费 `Frame`，不碰其它层。

**依赖方向对照**

```
V1:  Widget ──owns──> Component
       ▲                  │
       └──GetChartWidget──┘        ← 双向环
           Component ──GetParentModule──> Module（按 FName 线性扫）

V2:  Widget ─> Spec ─┐
          └──> Data ─┴─> FrameBuilder ─> Frame ─> Render（只读）
                                          │
                                          └─> Space（正/逆映射）
     单向、无环；Render 拿不到 Widget，只能读 Frame
```

---

## 5. 规模对照

| 指标 | V1 | V2 阶段 A | 变化 |
| --- | -- | -- | -- |
| 文件数 | 24 | 38 | +58% |
| 代码行 | 870 | 3,000–3,800 | ×3.5–4.4 |
| 目录层数 | 3（Core/Feature/Widget） | 7（Core/Spec/Data/Layout/Frame/Render/Widget） | +4 |
| 能画文字 | ❌ | ✅ | — |
| 能画轴/刻度/网格 | ❌ | ✅ | — |
| 运行时可增删系列 | ❌ | ✅ | — |
| 配置可序列化/下发 | ❌ | ✅ | — |
| 逆映射（tooltip 前提） | ❌ | ✅ | — |

**为什么行数涨这么多**：V1 的 870 行里，真正属于"图表"的是坐标映射（46 行）和一条折线（81 行），
其余是骨架和脚手架。V2 新增的三个大块——`Layout`（约 850 行）、`Frame`（约 680 行）、`Spec`（约 310 行）
——在 V1 里**完全不存在**。这不是"同一个东西写得更啰嗦"，是补上了原本缺的三层。
换个说法：V1 用 870 行买到一个能画线的画布，V2 用 ~3,500 行买到一套图表系统。

---

## 6. 迁移顺序（每步都能编译、可验证）

| 批次 | 内容 | 涉及文件 | 验证方式 |
| --- | --- | --- | --- |
| **C1 重排** | 目录与改名，不改任何行为 | Build.cs、Canvas 移动、DrawContext→PaintContext、Utils→Math、清 ini | 编译通过 + 折线照旧显示 |
| **C2 图元** | 扩 PaintContext（文字/圆/多边形/虚线/裁剪/分层）+ Types | Core/* | 无人调用，编译通过 |
| **C3 配置** | Spec 族 + Diagnostics（纯新增，零依赖） | Spec/* | 能存盘/读盘 |
| **C4 数学** | Space / Ticks / Layout / Math（纯 C++，无 Slate 依赖） | Layout/* | **写单测**：刻度、映射、布局 |
| **C5 帧** | Frame + FrameBuilder，接入 SChartCanvas，出轴/网格/标题 | Frame/*、Canvas | 看图：有轴有标签 |
| **C6 数据** | Buffer（含环形）+ DataSource | Data/* | 追加 1 万点不崩 |
| **C7 协调** | Reconciler + Widget API + 渲染器池 + 渲染器降级 | Frame/Reconciler、Widget、Render | **2→3 条线不打包不闪** |
| **C8 清场** | 删 ChartModule / ChartGlobalOption / 旧 Axis 路径 | 3 个删除项 | 全量回归 |

**C7 是验收点**：跑通它，你最初提的那个需求就闭环了。C1–C6 都是为它铺路。

---

## 7. 一句话总结

V1 和 V2 的差别不在文件多少，而在**目录按什么切**：
V1 按"谁负责什么角色"切（Modules / Components），所以配置只能设计期定死；
V2 按"数据流走到哪一步"切（Spec → Data → Layout → Frame → Render），
所以配置可以是一个值、在运行时被整体替换或者逐条打补丁。

**目录结构本身就是架构。** 换目录不是整理文件，是把"谁是权威状态"这件事重新摆正。
