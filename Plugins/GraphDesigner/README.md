# GraphDesigner 插件文档

GraphDesigner 是一个 Unreal Engine Runtime 插件，用于在 UMG 中创建、编辑、保存、读取组网图/拓扑图。插件使用 UMG 暴露蓝图控件，内部使用 Slate 绘制画布、图元、Pin、连线和右侧编辑面板，并使用 JSON 保存图纸数据。

## 一、树状架构图

```text
GraphDesigner
├─ GraphDesigner.uplugin
│  └─ 插件描述文件，声明 Runtime 模块 GraphDesigner
│
├─ Source
│  └─ GraphDesigner
│     ├─ GraphDesigner.Build.cs
│     │  └─ 模块依赖配置：Core、Engine、UMG、Slate、Json、ImageWrapper 等
│     │
│     ├─ Public
│     │  ├─ GraphDesigner.h
│     │  │  └─ 模块入口声明、日志分类声明
│     │  ├─ GraphDesignerTypes.h
│     │  │  └─ 图元、Pin、连线、答案数组、模板、文档数据结构
│     │  ├─ GraphDocument.h
│     │  │  └─ 图纸数据模型 UObject，负责增删改查、序列化和答案校验
│     │  ├─ GraphCanvasWidget.h
│     │  │  └─ 运行时画布 UUserWidget，只显示/操作画布，暴露蓝图接口
│     │  ├─ GraphDesignerWidget.h
│     │  │  └─ 完整编辑器 UUserWidget，包含左侧图元、画布、右侧面板
│     │  └─ SGraphDesignerCanvas.h
│     │     └─ Slate 画布控件声明，负责绘制和交互
│     │
│     └─ Private
│        ├─ GraphDesigner.cpp
│        │  └─ 模块启动/关闭实现
│        ├─ GraphDocument.cpp
│        │  └─ 图纸数据模型实现，包含 JSON 保存读取和 pin id 刷新
│        ├─ GraphCanvasWidget.cpp
│        │  └─ 运行时 UMG 包装层实现
│        ├─ GraphDesignerWidget.cpp
│        │  └─ 完整编辑器 UMG 包装层实现
│        ├─ SGraphDesignerCanvas.cpp
│        │  └─ Slate 画布绘制、拖拽、缩放、选择、连线、复制粘贴等核心逻辑
│        ├─ SGraphDesignerInspector.h/.cpp
│        │  └─ 右侧细节面板，编辑节点、Pin、连线属性
│        ├─ SGraphDesignerAnswerPanel.h/.cpp
│        │  └─ 右侧答案数组面板，编辑 (StartPinId, EndPinId) 列表
│        ├─ SGraphDesignerPalette.h/.cpp
│        │  └─ 左侧图元面板，提供可拖拽模板
│        └─ GraphDesignerDragDropOp.h
│           └─ 图元模板拖拽操作对象
```

## 二、文件作用说明

### 插件与模块

| 文件 | 作用 |
| --- | --- |
| `GraphDesigner.uplugin` | 插件描述文件。声明插件名称、版本、类型为 Runtime，并指定模块 `GraphDesigner` 在默认阶段加载。 |
| `Source/GraphDesigner/GraphDesigner.Build.cs` | Unreal 模块构建配置。公共依赖包含 `Core`、`CoreUObject`、`Engine`、`Json`、`JsonUtilities`、`UMG`；私有依赖包含 `Slate`、`SlateCore`、`InputCore`、`ImageWrapper`、`RenderCore`。 |
| `Public/GraphDesigner.h` | 模块接口声明，定义 `FGraphDesignerModule` 和日志分类 `LogGraphDesigner`。 |
| `Private/GraphDesigner.cpp` | 模块入口实现，启动和关闭时输出日志，并通过 `IMPLEMENT_MODULE` 注册模块。 |

### 数据结构与文档模型

| 文件 | 作用 |
| --- | --- |
| `Public/GraphDesignerTypes.h` | 定义所有可序列化结构和枚举，包括图元类型、Pin 文本位置、连线寻路模式、图元、Pin、连线、答案连接、图元模板、文档保存结构等。 |
| `Public/GraphDocument.h` | 声明 `UGraphDocument`，它是图纸数据模型，保存 `Elements`、`Connections`、`RequiredConnections`。 |
| `Private/GraphDocument.cpp` | 实现图纸数据的增删改查、Pin ID 自动生成/刷新、连线合法性检查、答案数组管理、JSON 序列化、文件保存读取。 |

### UMG 对外控件

| 文件 | 作用 |
| --- | --- |
| `Public/GraphCanvasWidget.h` / `Private/GraphCanvasWidget.cpp` | 运行时画布控件。适合在考试/展示界面使用，提供创建图纸、加载图纸、添加图元、添加 Pin、添加连线、保存读取、答案数组读写和 `IsRequiredConnection` 校验等蓝图接口。 |
| `Public/GraphDesignerWidget.h` / `Private/GraphDesignerWidget.cpp` | 完整编辑器控件。包含左侧图元面板、中间画布、右侧细节面板、答案数组面板、保存/读取按钮。适合制图、配置答案、调试图纸。 |

### Slate UI 与交互

| 文件 | 作用 |
| --- | --- |
| `Public/SGraphDesignerCanvas.h` / `Private/SGraphDesignerCanvas.cpp` | 核心画布。负责绘制背景网格、图元、Pin、连线、选择框；处理鼠标拖拽、缩放、平移、框选、复制粘贴、手动折线连线、点击选中连线等交互。 |
| `Private/SGraphDesignerInspector.h` / `.cpp` | 右侧细节面板。根据当前选中对象显示不同编辑项：节点文本、颜色、字体、Pin 参数、连线颜色和粗细等。 |
| `Private/SGraphDesignerAnswerPanel.h` / `.cpp` | 右侧答案数组面板。提供 `(startid, endid)` 列表的添加、删除、编辑，并同步写入 `UGraphDocument::RequiredConnections`。 |
| `Private/SGraphDesignerPalette.h` / `.cpp` | 左侧图元面板。按“设备图元、基础图形、连线图元、标注与连接”等类别展示模板，并支持拖拽到画布。 |
| `Private/GraphDesignerDragDropOp.h` | Slate 拖拽操作对象。保存被拖拽的 `FGraphDesignerNodeTemplate`，画布 Drop 时根据模板创建节点或 Pin。 |

## 三、核心架构说明

插件整体采用“数据模型 + Slate 画布 + UMG 包装”的结构。

### 1. 数据层：`UGraphDocument`

`UGraphDocument` 是图纸的唯一数据源，主要保存三类数据：

- `Elements`：所有图元节点，例如矩形、圆形、文本、图片、设备、Pin 图元等。
- `Connections`：用户实际连出来的线，每条线包含 `StartPinId`、`EndPinId`、`Color`、`Thickness`、`RoutePoints`。
- `RequiredConnections`：当前组网图要求考生连出的答案数组，每个元素是 `(StartPinId, EndPinId)`。

所有保存、读取、蓝图校验都围绕 `UGraphDocument` 进行。这样可以保证编辑器、运行时画布和 JSON 文件看到的是同一份图纸数据。

### 2. 表现层：`SGraphDesignerCanvas`

`SGraphDesignerCanvas` 是 Slate 控件，直接负责绘制和交互。

主要职责：

- 绘制背景和网格。
- 绘制各种基础图形。
- 绘制 Pin 和 Pin 文本。
- 绘制连接线和连接线预览。
- 鼠标点击选择节点、Pin、连线。
- 鼠标右键/中键平移画布。
- 鼠标滚轮缩放画布。
- 拖拽图元模板到画布生成图元。
- 点击 Pin 创建连接线。
- 支持手动折线路由点。
- 支持框选、复制、粘贴、删除。

画布不直接负责保存文件，而是修改 `UGraphDocument` 的数据，然后调用 Slate 的 `Invalidate` 触发重绘。

### 3. 编辑 UI 层：`UGraphDesignerWidget`

`UGraphDesignerWidget` 是完整编辑器控件，适合放在制图界面。

布局结构：

```text
UGraphDesignerWidget
├─ 左侧：SGraphDesignerPalette
├─ 中间：SGraphDesignerCanvas
└─ 右侧：SVerticalBox
   ├─ SGraphDesignerInspector
   ├─ SGraphDesignerAnswerPanel
   ├─ 保存图纸按钮
   └─ 读取图纸按钮
```

它负责把 Slate 的回调连接到文档模型，例如：

- Inspector 修改节点颜色 -> 调用 `SetElementFillColor`。
- Inspector 修改连线粗细 -> 调用 `UGraphDocument::UpdateConnection`。
- 答案面板修改答案项 -> 调用 `UGraphDocument::UpdateRequiredConnection`。
- 读取图纸成功后 -> 刷新画布和答案面板。

### 4. 运行 UI 层：`UGraphCanvasWidget`

`UGraphCanvasWidget` 是轻量画布控件，适合考试运行界面。

它只包含画布，不包含左侧 Palette、右侧 Inspector 和保存读取按钮。它暴露蓝图接口，方便关卡蓝图或 UMG 蓝图调用：

- 加载指定图纸。
- 获取图纸文档。
- 添加图元、Pin、连线。
- 判断考生连线是否在答案数组中。

## 四、使用到的算法与技术点

### 1. Slate 自绘技术

插件使用 `FSlateDrawElement` 直接绘制图形：

- `MakeBox` 绘制背景、图片、选择框等矩形区域。
- `MakeLines` 绘制普通线、折线、边框和连接线。
- 自定义多边形点集绘制三角形、菱形、五边形、六边形、星形等。
- 通过 FontMeasure 计算多行文本尺寸并绘制文本。

这种方式比纯 UMG Widget 更适合画布类控件，因为图元数量和交互状态都可以集中在一个 Slate 控件中处理。

### 2. 坐标转换

画布维护两个核心视口参数：

- `Zoom`：缩放比例。
- `CanvasPan`：画布平移量。

坐标转换公式：

```text
GraphToScreen(GraphPosition) = GraphPosition * Zoom + CanvasPan
ScreenToGraph(ScreenPosition) = (ScreenPosition - CanvasPan) / Zoom
```

所有绘制、命中检测、拖拽移动都基于这套坐标系统。

### 3. 命中检测算法

画布根据鼠标位置判断当前点击对象：

- 图元命中：使用 `FBox2D` 判断鼠标是否在图元矩形范围内。
- Pin 命中：计算鼠标点到 Pin 中心的距离，小于命中半径即选中。
- 连线命中：将连接线拆成多段线段，计算鼠标点到每条线段的距离，小于容差即选中。

连线命中核心思想：

```text
for each segment in connection:
    distance = DistancePointToSegment(mouse, segment.start, segment.end)
    if distance <= tolerance:
        hit
```

### 4. 正交折线路由

插件支持两种连线路由模式：

- `AutoOrthogonal`：自动正交折线。
- `ManualRoutePoints`：手动点击添加路由点，再连接到终点。

正交折线通过起点和终点构造水平/垂直折线点，使连接线更符合网络拓扑图风格。

手动路由时，用户在画布上点击添加中间路由点。系统会根据当前最后一个锚点到新点/终点之间生成正交线段，并保存到 `Connection.RoutePoints`。

### 5. 复制粘贴与 ID 重建

复制图元时，插件会保存选中的 `Elements` 和相关 `Connections`。粘贴时会：

- 为新图元生成新的 Element ID。
- 为新 Pin 生成新的 Pin ID。
- 使用映射表把旧连接引用替换成新 Pin ID。
- 给粘贴内容添加偏移，避免和原图重叠。

### 6. Pin ID 生成与刷新

Pin ID 基于所属节点文本和 Pin 文本生成：

```text
OwnerText_PinText
```

如果为空，会使用 fallback；如果重复，会自动追加 `_2`、`_3` 等后缀。节点文本或 Pin 文本变化时，`UGraphDocument` 会刷新 Pin ID，并同步替换普通连接和答案数组里的引用。

### 7. JSON 序列化

插件使用 `FJsonObjectConverter` 将 `FGraphDesignerDocumentData` 序列化为 JSON。

保存内容包含：

- `Version`
- `DocumentId`
- `Elements`
- `Connections`
- `RequiredConnections`

默认保存目录：

```text
Saved/Graphs
```

如果保存名没有扩展名，会自动追加 `.json`。

### 8. 蓝图接口

核心 UObject 类使用 `UCLASS`、`USTRUCT`、`UENUM`、`UFUNCTION`、`UPROPERTY` 暴露给蓝图。运行时可以在蓝图中调用：

- `LoadSavedGraphByName`
- `AddConnection`
- `DisconnectConnectionByPins`
- `GetRequiredConnections`
- `IsRequiredConnection`

## 五、使用方法

### 1. 启用插件

插件位于：

```text
Plugins/GraphDesigner
```

确认项目 `.uproject` 中启用了插件，或者在 Unreal Editor 的插件面板中启用 `GraphDesigner`。

### 2. 制作图纸

推荐在制图/编辑界面使用 `UGraphDesignerWidget`。

使用步骤：

1. 在 UMG 蓝图中添加 `GraphDesignerWidget`。
2. 运行界面后，左侧选择图元模板。
3. 将图元拖拽到中间画布。
4. 选中节点后，在右侧细节面板编辑文本、颜色、字体、Pin。
5. 点击 Pin 开始连线，再点击另一个 Pin 完成连线。
6. 左键点击连线，可在右侧编辑连线颜色和粗细。
7. 在右侧“答案数组”中添加需要考生连接的 `(startid, endid)`。
8. 点击“保存图纸”，输入图纸名称保存。

### 3. 配置答案数组

答案数组每一项是：

```text
(StartPinId, EndPinId)
```

例如：

```text
Server_ETH0, Switch_GE0
Router_WAN, Switch_GE1
```

注意：

- 当前判断是有方向匹配：传入的 `StartPinId` 和 `EndPinId` 必须与答案项一致。
- 如果需要无方向判断，可以在业务层调用两次：

```text
IsRequiredConnection(A, B) || IsRequiredConnection(B, A)
```

### 4. 考试/运行时加载图纸

推荐在考试界面使用 `UGraphCanvasWidget`。

常见流程：

1. 在 UMG 蓝图中添加 `GraphCanvasWidget`。
2. 设置 `SavedGraphName`。
3. 勾选 `bLoadSavedGraphOnConstruct`，或手动调用 `LoadSavedGraphByName(GraphName)`。
4. 考生点击 Pin 创建连接线。
5. 监听 `OnConnectionCreated` 或 `OnConnectionCreatedByName`。
6. 使用 `IsRequiredConnection(StartPinId, EndPinId)` 判断该连接是否在答案数组中。

蓝图逻辑示例：

```text
OnConnectionCreated(StartPinId, EndPinId)
    -> IsRequiredConnection(StartPinId, EndPinId)
        true  : 记为正确连线
        false : 记为错误连线
```

### 5. 常用蓝图接口

#### `UGraphCanvasWidget`

| 接口 | 说明 |
| --- | --- |
| `CreateNewGraph()` | 创建空白图纸。 |
| `LoadSavedGraphByName(GraphName)` | 从 `Saved/Graphs` 读取图纸。 |
| `SaveGraph(GraphName)` | 保存当前图纸。 |
| `AddElement(Type, Position, Size)` | 添加图元。 |
| `AddPinToElement(ElementId, RelativePosition, PinText)` | 给图元添加 Pin。 |
| `AddConnection(StartPinId, EndPinId)` | 添加连接线。 |
| `DisconnectConnectionByPins(StartPinId, EndPinId)` | 删除两个 Pin 之间的连接。 |
| `SetRequiredConnections(Array)` | 设置答案数组。 |
| `GetRequiredConnections()` | 获取答案数组。 |
| `AddRequiredConnection(StartPinId, EndPinId)` | 添加答案项。 |
| `UpdateRequiredConnection(Index, StartPinId, EndPinId)` | 修改答案项。 |
| `RemoveRequiredConnectionAt(Index)` | 删除答案项。 |
| `IsRequiredConnection(StartPinId, EndPinId)` | 判断传入连接是否存在于答案数组。 |

#### `UGraphDesignerWidget`

| 接口 | 说明 |
| --- | --- |
| `SetNodeTemplates(NodeTemplates)` | 替换左侧图元模板。 |
| `ResetToDefaultTemplates()` | 恢复默认模板，包括服务器、交换机、路由器、基础形状等。 |
| `SetElementText(ElementId, Text)` | 修改节点文本。 |
| `SetElementFillColor(ElementId, Color)` | 修改节点填充色。 |
| `SetElementBorderColor(ElementId, Color)` | 修改节点边框色。 |
| `SetRequiredConnections(Array)` | 设置答案数组。 |
| `GetRequiredConnections()` | 获取答案数组。 |
| `IsRequiredConnection(StartPinId, EndPinId)` | 判断答案连接。 |
| `SaveGraph(GraphName)` | 保存图纸。 |
| `LoadGraph(GraphName)` | 读取图纸。 |

### 6. 默认图元模板

`UGraphDesignerWidget::ResetToDefaultTemplates()` 会创建以下默认模板：

- 设备图元：服务器、交换机、路由器。
- 基础图形：矩形、圆角矩形、圆形、椭圆、三角形、菱形、多边形、星形等。
- 连线图元：线段、箭头。
- 标注与连接：文本、图片、连接点 Pin。

可以通过 `SetNodeTemplates` 自定义模板列表。

## 六、扩展建议

### 1. 无方向答案判断

当前 `IsRequiredConnection(StartPinId, EndPinId)` 是精确方向判断。如业务需要“只要两个 Pin 相连就算正确”，可以新增接口：

```text
IsRequiredConnectionUndirected(StartPinId, EndPinId)
```

内部判断：

```text
(A == Start && B == End) || (A == End && B == Start)
```

### 2. 答案自动生成

可以增加一个按钮，将当前实际连线 `Connections` 一键写入 `RequiredConnections`，减少手动输入答案数组的工作量。

### 3. 连线样式扩展

目前连线已支持颜色、粗细、实线/虚线。后续可以扩展：

- 箭头方向。
- 错误连线高亮。
- 正确连线高亮。

### 4. 图纸版本兼容

`FGraphDesignerDocumentData` 已包含 `Version` 字段。后续如果 JSON 结构变化，可以根据版本号做兼容升级。



