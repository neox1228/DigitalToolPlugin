# NXCharts 插件开发文档

## 1. 插件概述

NXCharts 是一个 Unreal Engine Runtime 图表插件，目标是在 UMG 中提供可配置、可扩展的图表控件。当前实现以 `UChartWidget` 作为蓝图和 UMG 的入口，通过 Slate 的 `SChartCanvas` 完成底层绘制，并使用模块 `UChartModule` 与组件 `UChartComponent` 拆分图表能力。

当前插件已经具备的基础能力：

- UMG 控件入口：`UChartWidget`
- Slate 自绘画布：`SChartCanvas`
- 图表全局配置：`FChartGlobalOption`
- 图表主题数据资产：`UChartTheme`
- 图表模块基类：`UChartModule`
- 笛卡尔坐标系模块：`UCartesianCoordModule`
- 图表组件基类：`UChartComponent`
- 坐标轴组件：`UAxisComponent`
- 折线系列组件：`UlineSeriesComponent`
- 点击、悬停命中测试的基础框架

> 注意：按当前工作区代码状态，`AxisComponent.cpp` 中 `UAxisComponent::CalculateAxisPoints` 的实现未完成，源码存在语法中断，插件会编译失败。修复该文件后再进行完整编译和蓝图测试。

## 2. 使用技术

### Unreal Engine 插件系统

插件描述文件为 `NXCharts.uplugin`：

- 插件名：`NXCharts`
- 模块名：`NXCharts`
- 模块类型：`Runtime`
- 加载阶段：`Default`
- `CanContainContent: true`，允许插件携带 Content 资源

### C++ Runtime Module

模块入口在：

- `Source/NXCharts/Public/NXCharts.h`
- `Source/NXCharts/Private/NXCharts.cpp`

`FNXChartsModule` 继承 `IModuleInterface`，目前 `StartupModule` 和 `ShutdownModule` 为空实现。

### UMG

`UChartWidget` 继承自 `UWidget`，用于在 UMG Designer 和蓝图中作为图表控件使用。它负责保存全局配置、模块列表、组件列表，并创建底层 Slate 控件。

### Slate 自绘

`SChartCanvas` 继承自 `SLeafWidget`，通过重写 `OnPaint` 使用 Slate 绘制 API 输出图表元素。绘制元素通过 `ChartDrawContext` 封装。

### UObject 反射和蓝图暴露

插件中的配置、模块、组件大量使用 Unreal 反射宏：

- `UCLASS`
- `USTRUCT`
- `UENUM`
- `UPROPERTY`
- `UFUNCTION`
- `BlueprintType`
- `Blueprintable`
- `BlueprintReadWrite`
- `BlueprintCallable`
- `BlueprintAssignable`
- `EditInlineNew`
- `Instanced`

这使得图表对象可以在 UMG、Details 面板和蓝图中配置或调用。

### Slate 绘制 API

绘制主要使用：

- `FSlateDrawElement::MakeLines`
- `FSlateDrawElement::MakeBox`
- `FSlateWindowElementList`
- `FGeometry`
- `FPaintGeometry`

## 3. 模块依赖

`NXCharts.Build.cs` 中声明的依赖如下：

### PublicDependencyModuleNames

- `Core`
- `UMG`
- `InputCore`

### PrivateDependencyModuleNames

- `CoreUObject`
- `Engine`
- `Slate`
- `SlateCore`

这些依赖说明插件面向 Runtime 使用，并且依赖 UMG 作为外部控件入口、Slate/SlateCore 作为实际绘制层。

## 4. 总体架构

NXCharts 当前采用三层结构：

```text
UMG / Blueprint
    |
    v
UChartWidget
    |
    | RebuildWidget()
    v
SChartCanvas
    |
    | OnPaint()
    v
ChartDrawContext
    |
    +--> UChartModule::CollectDrawElements()
    +--> UChartComponent::CollectDrawElements()
```

### 核心职责划分

`UChartWidget`：

- 作为 UMG 控件暴露给蓝图
- 持有 `GlobalOption`
- 持有 `Modules`
- 持有 `Components`
- 提供动态添加模块和组件的蓝图接口
- 提供刷新图表接口
- 提供图表级点击事件

`SChartCanvas`：

- 负责实际 Slate 控件生命周期
- 负责 `OnPaint` 绘制调度
- 负责鼠标移动、鼠标点击、滚轮事件入口
- 负责命中测试调度

`ChartDrawContext`：

- 封装 Slate 绘制参数
- 管理绘制层级
- 提供线段、矩形等基础绘制函数

`UChartModule`：

- 表示图表级功能模块，例如坐标系、图例、网格、缩放、数据域管理
- 可提供绘图区范围
- 可响应组件数据变化
- 可广播模块状态更新

`UChartComponent`：

- 表示具体可绘制图表元素，例如坐标轴、折线、柱状图、散点图、标签、提示框
- 可绑定到某个模块
- 可参与绘制
- 可参与命中测试
- 可向父模块通知数据变化

## 5. 绘制流程

`UChartWidget::RebuildWidget` 创建 `SChartCanvas`：

```cpp
ChartCanvas = SNew(SChartCanvas, this);
return ChartCanvas.ToSharedRef();
```

`SChartCanvas::OnPaint` 是绘制入口：

1. 检查 `Owner` 是否有效。
2. 创建 `ChartDrawContext`。
3. 遍历 `Owner->Modules`，调用每个模块的 `CollectDrawElements`。
4. 遍历 `Owner->Components`，对可见组件调用 `CollectDrawElements`。
5. 返回新的 Slate LayerId。

当前绘制顺序是：

```text
模块绘制 -> 组件绘制 -> 预留高亮覆盖层
```

因此坐标系背景、网格等适合放在 Module 中，折线、轴线、数据点等适合放在 Component 中。

## 6. 交互流程

`SChartCanvas` 当前实现了三个交互入口：

- `OnMouseMove`
- `OnMouseButtonDown`
- `OnMouseWheel`

### 悬停命中

鼠标移动时：

1. 将屏幕坐标转换为控件局部坐标。
2. 遍历所有可见组件。
3. 调用组件的 `HitTest(LocalPos, MyGeometry)`。
4. 记录第一个命中的组件索引和数据项索引。
5. 命中结果变化时触发重绘。

### 点击

鼠标左键点击时，当前代码会检查上一次悬停命中的组件，但广播事件代码仍被注释：

```cpp
// Owner->OnChartItemClicked.Broadcast(HitComp, LastHitItemIndex);
```

如需蓝图接收点击事件，需要恢复该广播，并视需求同时广播组件自己的 `OnComponentClicked`：

```cpp
Owner->OnChartItemClicked.Broadcast(HitComp, LastHitItemIndex);
HitComp->OnComponentClicked.Broadcast(HitComp, LastHitItemIndex);
```

### 滚轮

`OnMouseWheel` 当前为空实现，后续可用于坐标缩放、平移或数据窗口调整。

## 7. 文件功能说明

### 插件根目录

| 文件或目录 | 功能 |
| --- | --- |
| `NXCharts.uplugin` | 插件描述文件，声明 Runtime 模块和插件元信息 |
| `Config/DefaultNXCharts.ini` | 插件默认配置和 CoreRedirects |
| `Resources/Icon128.png` | 插件图标 |
| `Content/` | 插件资源目录，当前未发现资源文件 |
| `Binaries/` | 编译产物，不建议手工修改 |
| `Intermediate/` | Unreal 中间产物，不建议提交或手工修改 |
| `Source/NXCharts/` | 插件源码目录 |

### Source/NXCharts

| 文件 | 功能 |
| --- | --- |
| `NXCharts.Build.cs` | 声明模块依赖、PCH 策略、Include Path |
| `Public/NXCharts.h` | Runtime 模块类声明 |
| `Private/NXCharts.cpp` | Runtime 模块实现和 `IMPLEMENT_MODULE` |

### Core

| 文件 | 功能 |
| --- | --- |
| `Public/Core/ChartDrawContext.h` | Slate 绘制上下文封装，提供 `DrawLine`、`DrawRect` |
| `Public/Core/ChartGlobalOption.h` | 图表全局外观配置，支持蓝图编辑 |
| `Public/Core/ChartTheme.h` | 图表主题 DataAsset，定义颜色、字体大小等默认样式 |
| `Public/Core/ChartUtils.h` | 图表工具函数声明和内联函数 |
| `Private/Core/ChartUtils.cpp` | 点到线段距离等工具函数实现 |
| `Private/Core/ChartCanvas.cpp` | `SChartCanvas` 的绘制和交互实现 |
| `Private/Core/ChartTheme.cpp` | 主题类实现文件，目前无额外逻辑 |

### Widget

| 文件 | 功能 |
| --- | --- |
| `Public/Widget/ChartWidget.h` | UMG 图表控件声明，蓝图入口 |
| `Private/Widget/ChartWidget.cpp` | 创建 Slate 控件、刷新、动态添加/查找模块和组件 |

### Feature/Modules

| 文件 | 功能 |
| --- | --- |
| `Public/Feature/Modules/ChartModule.h` | 图表模块基类，提供模块名、绘制、数据变化通知、绘图区接口 |
| `Private/Feature/Modules/ChartModule.cpp` | 模块基类实现文件，目前无额外逻辑 |
| `Public/Feature/Modules/CartesianCoordModule.h` | 笛卡尔坐标系模块，维护 X/Y 范围和绘图区 |
| `Private/Feature/Modules/CartesianCoordModule.cpp` | 绘制绘图区背景、提供带 Padding 的绘图区 |

### Feature/Components

| 文件 | 功能 |
| --- | --- |
| `Public/Feature/Components/ChartComponent.h` | 图表组件基类，提供名称、父模块绑定、可见性、绘制、命中和事件 |
| `Private/Feature/Components/ChartComponent.cpp` | 父 Widget/父 Module 查找和数据变化通知 |
| `Public/Feature/Components/AxisComponent.h` | 坐标轴组件声明，支持 X/Y 轴、颜色、线宽和轴范围 |
| `Private/Feature/Components/AxisComponent.cpp` | 坐标轴绘制实现，当前未完成且存在语法错误 |
| `Public/Feature/Components/LineSeriesComponent.h` | 折线系列组件声明，支持数据点、线色、线宽 |
| `Private/Feature/Components/LineSeriesComponent.cpp` | 数据点映射、折线绘制、线段命中测试 |

## 8. 核心类和接口介绍

### UChartWidget

头文件：`Public/Widget/ChartWidget.h`

主要属性：

```cpp
FChartGlobalOption GlobalOption;
TArray<UChartModule*> Modules;
TArray<UChartComponent*> Components;
```

蓝图接口：

```cpp
void RefreshChart();
UChartModule* AddModule(TSubclassOf<UChartModule> ModuleClass);
UChartComponent* AddComponent(TSubclassOf<UChartComponent> ComponentClass, FName ModuleName = NAME_None);
UChartModule* GetModuleByName(FName ModuleName) const;
UChartComponent* GetComponentByName(FName ComponentName) const;
```

蓝图事件：

```cpp
OnChartItemClicked(UChartComponent* Component, int32 ItemIndex)
```

当前注意点：

- `GetComponentByName` 当前实现没有比较 `ComponentName`，会直接返回第一个组件。应修复为按名称匹配。
- `OnChartItemClicked` 在 `SChartCanvas::OnMouseButtonDown` 中尚未实际广播。

### UChartModule

头文件：`Public/Feature/Modules/ChartModule.h`

关键属性：

```cpp
FName ModuleName;
```

关键虚函数：

```cpp
virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry);
virtual void NotifyComponentDataChanged(UChartComponent* Component);
virtual FBox2D GetPlotBounds(const FGeometry& AllottedGeometry) const;
```

事件：

```cpp
OnModuleUpdated
```

适合扩展的功能：

- 坐标系
- 图例
- 网格
- 标题
- 数据缩放
- Tooltip
- 多轴管理

### UCartesianCoordModule

头文件：`Public/Feature/Modules/CartesianCoordModule.h`

关键属性：

```cpp
FVector2D XAxisRange = FVector2D(0, 100);
FVector2D YAxisRange = FVector2D(0, 100);
FVector2D CachedXRange;
FVector2D CachedYRange;
```

关键接口：

```cpp
FBox2D GetPlotBounds(const FGeometry& AllottedGeometry) const;
FBox2D GetPaddedPlotBounds(const FGeometry& AllottedGeometry) const;
```

当前实现：

- 使用固定内边距 `Left=50, Top=30, Right=30, Bottom=50`
- 绘图区用白色矩形填充
- 自动范围更新逻辑尚未实现

建议后续将固定内边距改为读取 `UChartWidget::GlobalOption.Padding`。

### UChartComponent

头文件：`Public/Feature/Components/ChartComponent.h`

关键属性：

```cpp
FName ComponentName;
FName ParentModuleName;
bool bVisible = true;
```

关键虚函数：

```cpp
virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry);
virtual int32 HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const;
virtual void OnModuleChanged();
```

辅助函数：

```cpp
void NotifyDataChanged();
UChartWidget* GetChartWidget() const;
UChartModule* GetParentModule() const;
```

事件：

```cpp
OnComponentClicked(UChartComponent* Component, int32 ItemIndex)
```

### UlineSeriesComponent

头文件：`Public/Feature/Components/LineSeriesComponent.h`

关键属性：

```cpp
TArray<FVector2D> DataPoints;
FLinearColor LineColor = FLinearColor::Green;
float LineWidth = 2.0f;
```

当前行为：

- 从父模块读取 `UCartesianCoordModule`
- 使用坐标系的 PlotBounds 和 X/Y 范围把数据坐标映射为像素坐标
- 使用 `ChartDrawContext::DrawLine` 绘制折线
- 使用 `ChartUtils::PointToSegmentDistance` 做折线命中测试

数据坐标到像素坐标转换逻辑：

```cpp
float Px = ChartUtils::MapValue(Pt.X, XRange.X, XRange.Y, PlotBounds.Min.X, PlotBounds.Max.X);
float Py = ChartUtils::MapValue(Pt.Y, YRange.X, YRange.Y, PlotBounds.Max.Y, PlotBounds.Min.Y);
```

Y 轴映射使用 `PlotBounds.Max.Y -> PlotBounds.Min.Y`，这是因为 Slate 局部坐标的 Y 方向向下，而常见图表数据 Y 方向向上。

### UAxisComponent

头文件：`Public/Feature/Components/AxisComponent.h`

关键属性：

```cpp
EChartAxisType AxisType;
FLinearColor AxisColor;
float AxisThickness;
bool bAutoAdjustRange;
FChartAxisRange AxisRange;
```

当前状态：

- 头文件接口已定义
- `CollectDrawElements` 会调用 `CalculateAxisPoints`
- `CalculateAxisPoints` 未完成
- `AxisThickness` 没有默认值，建议设置默认值，例如 `1.0f`

### ChartDrawContext

头文件：`Public/Core/ChartDrawContext.h`

关键接口：

```cpp
int32 ReserveLayer();
void DrawLine(const TArray<FVector2D>& Points, const FLinearColor& Color, float Thickness = 1.0f, bool bAntialias = true);
void DrawRect(const FVector2D& TopLeft, const FVector2D& Size, const FLinearColor& Color);
```

扩展图表类型时，优先在这里增加通用绘制能力，例如：

- `DrawCircle`
- `DrawText`
- `DrawSpline`
- `DrawPolygon`
- `DrawDashedLine`

### ChartUtils

头文件：`Public/Core/ChartUtils.h`

当前函数：

```cpp
float MapValue(float Value, float InMin, float InMax, float OutMin, float OutMax);
float Distance(const FVector2D& A, const FVector2D& B);
float PointToSegmentDistance(const FVector2D& Point, const FVector2D& A, const FVector2D& B, FVector2D& OutClosestPoint);
```

适合继续加入：

- 数据范围计算
- Tick 自动分割
- 数值格式化
- 坐标 clamp
- 矩形/点/圆命中测试

## 9. 如何开发扩展

### 扩展一个新的图表组件

例如新增柱状图组件：

1. 在 `Public/Feature/Components/` 新建 `BarSeriesComponent.h`。
2. 在 `Private/Feature/Components/` 新建 `BarSeriesComponent.cpp`。
3. 继承 `UChartComponent`。
4. 使用 `UCLASS(Blueprintable, EditInlineNew)` 或至少 `UCLASS()`。
5. 添加蓝图可编辑的数据和样式属性。
6. 重写 `CollectDrawElements`。
7. 如需交互，重写 `HitTest`。
8. 在数据变化时调用 `NotifyDataChanged()` 并让 Widget 刷新。

示例骨架：

```cpp
UCLASS(Blueprintable, EditInlineNew)
class NXCHARTS_API UBarSeriesComponent : public UChartComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bar")
    TArray<FVector2D> DataPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bar")
    FLinearColor BarColor = FLinearColor::Blue;

    virtual void CollectDrawElements(ChartDrawContext& Ctx, const FGeometry& AllottedGeometry) override;
    virtual int32 HitTest(const FVector2D& LocalPos, const FGeometry& AllottedGeometry) const override;
};
```

实现时建议复用 `UCartesianCoordModule::GetPaddedPlotBounds` 和 `ChartUtils::MapValue`，保持不同图表类型的数据映射一致。

### 扩展一个新的模块

例如新增图例模块：

1. 继承 `UChartModule`。
2. 添加图例位置、字体、颜色等配置。
3. 在 `CollectDrawElements` 中绘制图例。
4. 如需感知组件数据变化，重写 `NotifyComponentDataChanged`。
5. 数据变化后调用 `BroadcastModuleUpdated()`。

模块更适合承载跨组件共享的上下文或辅助视觉元素，例如：

- 坐标系
- 网格线
- 图例
- Tooltip 状态
- 数据窗口和缩放状态
- 多轴布局

### 扩展绘制能力

如果多个组件都需要某类基础图元，应扩展 `ChartDrawContext`，而不是在每个组件里直接调用 Slate API。

建议新增接口示例：

```cpp
void DrawCircle(const FVector2D& Center, float Radius, const FLinearColor& Color);
void DrawText(const FString& Text, const FVector2D& Position, const FLinearColor& Color);
```

这样可以把 Slate 细节收敛到 Core 层，避免业务组件直接依赖复杂绘制参数。

### 扩展坐标范围计算

当前 `UCartesianCoordModule::NotifyComponentDataChanged` 只是广播更新，没有真正根据数据调整范围。建议实现逻辑：

1. 找到绑定到该模块的所有组件。
2. 从数据系列中读取 X/Y 最小值和最大值。
3. 给范围增加少量 padding。
4. 写入 `CachedXRange` 和 `CachedYRange`。
5. 调用 `BroadcastModuleUpdated()`。
6. 调用 `UChartWidget::RefreshChart()` 或由外部触发刷新。

建议把“提取组件数据范围”的能力设计为 `UChartComponent` 的虚函数，例如：

```cpp
virtual bool GetDataRange(FVector2D& OutXRange, FVector2D& OutYRange) const;
```

这样坐标模块不需要知道每一种系列组件的具体类型。

## 10. 蓝图中如何使用

### 在 UMG 中静态配置

1. 创建一个 Widget Blueprint。
2. 在 Designer 中添加 `ChartWidget` 控件。
3. 在 Details 面板配置 `GlobalOption`。
4. 在 `Modules` 数组中添加一个 `CartesianCoordModule`。
5. 设置该模块的 `ModuleName`，例如 `MainCoord`。
6. 在 `Components` 数组中添加：
   - `AxisComponent`，设置 `ParentModuleName = MainCoord`
   - `UlineSeriesComponent`，设置 `ParentModuleName = MainCoord`
7. 给折线组件填充 `DataPoints`。
8. 调用 `RefreshChart` 触发重绘。

### 在蓝图中动态创建

推荐流程：

```text
Event Construct
    -> AddModule(CartesianCoordModule)
    -> 设置返回模块的 ModuleName = "MainCoord"
    -> AddComponent(UlineSeriesComponent, "MainCoord")
    -> Cast to UlineSeriesComponent
    -> 设置 DataPoints / LineColor / LineWidth
    -> AddComponent(AxisComponent, "MainCoord")
    -> Cast to AxisComponent
    -> 设置 AxisType / AxisColor / AxisThickness
    -> RefreshChart
```

### 绑定点击事件

`UChartWidget` 提供了蓝图事件：

```text
OnChartItemClicked(Component, ItemIndex)
```

但当前 C++ 中点击广播被注释，必须先在 `SChartCanvas::OnMouseButtonDown` 中恢复广播，蓝图才能收到事件。

建议恢复：

```cpp
Owner->OnChartItemClicked.Broadcast(HitComp, LastHitItemIndex);
HitComp->OnComponentClicked.Broadcast(HitComp, LastHitItemIndex);
```

之后蓝图可绑定：

```text
Bind Event to OnChartItemClicked
    -> Component 表示命中的图表组件
    -> ItemIndex 表示命中的数据项索引
```

### 动态更新数据

蓝图中更新折线数据后，需要刷新图表：

```text
GetComponentByName("LineA")
    -> Cast to UlineSeriesComponent
    -> Set DataPoints
    -> RefreshChart
```

当前注意点：

- `GetComponentByName` 需要先修复按名称查找逻辑。
- 如果后续实现了组件数据变化通知，建议为系列组件增加蓝图函数，例如 `SetDataPoints`，内部调用 `NotifyDataChanged()` 和 `RefreshChart()`。

## 11. 当前限制和建议修复项

### 必须修复

1. `AxisComponent.cpp` 存在未完成语句，当前会导致编译失败。
2. `UChartWidget::GetComponentByName` 没有按 `ComponentName` 判断。
3. `SChartCanvas::OnMouseButtonDown` 未广播 `OnChartItemClicked`。

### 建议修复

1. `UlineSeriesComponent` 类名建议改为 `ULineSeriesComponent`，符合 Unreal 命名规范。若已被蓝图资源引用，需要配置 CoreRedirect。
2. `AxisThickness` 建议设置默认值。
3. `UCartesianCoordModule` 的 `XAxisRange`、`YAxisRange` 建议加 `UPROPERTY(EditAnywhere, BlueprintReadWrite)`，否则蓝图和 Details 面板不可直接编辑。
4. `GetPaddedPlotBounds` 建议使用 `GlobalOption.Padding`，而不是固定数值。
5. `FChartGlobalOption::DefaultFontName` 当前只是名字，不是真正的字体配置；后续绘制文本时可考虑 `FSlateFontInfo` 或字体资源引用。
6. `ChartDrawContext` 当前没有文字、圆点、虚线、填充多边形等图元绘制接口。
7. `UChartModule::OnModuleUpdated` 当前没有自动绑定到组件的 `OnModuleChanged`。
8. `RefreshChart` 当前只触发 Widget invalidation，不负责数据范围重算。

## 12. 推荐开发路线

建议按以下顺序推进：

1. 修复 `AxisComponent.cpp` 编译错误。
2. 修复 `GetComponentByName` 和点击事件广播。
3. 给 `UCartesianCoordModule` 的坐标范围加蓝图可编辑属性。
4. 将绘图区 Padding 接入 `GlobalOption.Padding`。
5. 增加 `ChartDrawContext::DrawText` 和 `DrawCircle`。
6. 给折线组件增加数据点圆点绘制和点级命中。
7. 实现坐标轴 Tick、标签和网格。
8. 抽象组件数据范围接口，完善自动范围。
9. 增加柱状图、散点图等更多系列组件。
10. 增加 Tooltip、Legend、Zoom/Pan 模块。

## 13. 编译和验证建议

修复当前语法问题后，建议执行：

```powershell
.\Engine\Build\BatchFiles\Build.bat MyProject2Editor Win64 Development "D:\WorkSpace\GitLab\DigitalToolPlugin\MyProject2.uproject" -WaitMutex
```

如果使用 Rider 或 Unreal Editor：

1. 重新生成项目文件。
2. 编译 `MyProject2Editor`。
3. 打开测试 UMG Widget。
4. 添加 `ChartWidget`。
5. 验证静态配置和蓝图动态创建两种路径。
6. 验证折线绘制、鼠标悬停命中、点击事件广播。

