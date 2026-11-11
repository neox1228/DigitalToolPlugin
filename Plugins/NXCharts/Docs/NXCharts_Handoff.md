# NXCharts 交接说明

> 新会话接手时先读这一份，再按需读 Manual / API / Usage。

## 1. 当前状态

- 版本：v0.3.x，**编译通过，14 组自动化测试全绿**
- 工程：`D:\WorkSpace\GitLab\DigitalToolPlugin`（UE 5.8）
- 插件：`Plugins/NXCharts`
- 分支：`main`，已与 origin 同步

## 2. 架构要点

```
Spec(JSON 文件) → FChartSpec → FChartFrameBuilder → FChartFrame(只读快照)
                                                          ↓
                                            SChartCanvas::OnPaint 按层遍历
                                                          ↓
                                    Painters/（一个组件一对文件）
```

- 配置：`SpecPath` 指向 Content 下 JSON，0.5 秒轮询自动重载（`bAutoReloadSpec` 可关）
- 绘制：`FChartPainter`（`GetLayer` / `IsEnabled` / `Paint`），无状态，开关读 `Frame.Spec`
- 缩放：`IChartZoom` + `FCartesianZoom`，画布只转发
- 加新绘制内容 = 一对文件 + `AllPainters.h` 一行 include + `Construct` 一行 Add

## 3. 构建与测试

```powershell
# 构建
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' `
  MyProject2Editor Win64 Development -Project="D:\WorkSpace\GitLab\DigitalToolPlugin\MyProject2.uproject" -WaitMutex

# 测试（必须先关编辑器，且不要加 -nullrhi）
UnrealEditor-Cmd.exe <uproject> '-ExecCmds="Automation RunTests NXCharts;Automation Quit"' `
  -unattended -nopause -nosplash -nosound -nop4 -windowed -ResX=640 -ResY=480 -stdout
```

## 4. 已知坑

| 坑 | 说明 |
| --- | --- |
| Unity Build 常量重名 | 匿名命名空间里的常量会跨 .cpp 冲突（`LegendMarkerSize` 撞过）。**统一加组件前缀** |
| PowerShell 读写中文 | 只用 `[System.IO.File]::ReadAllText/WriteAllText` + `UTF8Encoding($false)`；`Get-Content` 会按 ANSI 解码导致乱码 |
| git amend 丢路径 | `git commit --amend` 不带路径会把暂存区全部提交（曾误提交 143 个 Content 资产）。**一律带 `-- <paths>`** |
| 编辑器占用 | 编辑器开着时跑测试会加载 `-0001.dll`，结果为空。**先关编辑器** |
| 沙箱 | 写文件/构建/git 需要 `danger-full-access` |

## 5. 待办

### P0 · 修 X 轴重复 0 刻度

现象：缩放后 X 轴出现两个 `0s`，其中一个不落在步长整数格上。

排查方向：
- `Private/Layout/ChartTicks.cpp` → `MakeLinearTicks` 的 `ceil(Min/Step)*Step`
- `Private/Layout/ChartLayout.cpp` → 标签是否按格式化文本去重
- `Private/Widget/Zoom/CartesianZoom.cpp` → 缩放后 `Min` 可能是 `-1e-9`，需要近零吸附

建议：缩放结果加近零吸附 + 刻度按格式化文本去重。

### P1 · 拆分 GridAndAxesPainter

`Private/Widget/Painters/GridAndAxesPainter.cpp`（124 行）目前兼着标题、网格、轴线、刻度标签、轴标题。
应拆成 `TitlePainter` / `GridPainter` / `AxisPainter` 三个组件，其余照现有组件的写法。

### P2 · 可选

- 图例条目顺序可配
- `EChartSeriesKind` 的 Bar / Scatter 尚未实现渲染

## 6. 文档

| 文件 | 内容 |
| --- | --- |
| `NXCharts_Manual.md` | 架构、技术栈、值域与缩放设计 |
| `NXCharts_API.md` | 蓝图节点、Spec 字段、枚举、变更记录 |
| `NXCharts_Usage.md` | 使用步骤、JSON 配置、常见问题 |
| `Content/NXCharts/Demo.json` | 示例配置 |