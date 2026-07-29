# ExamJudge 使用说明

`ExamJudge` 是一个极简的实时小题完成判定引擎。

系统只做三件事：

1. 用 `Content` 保存当前小题最新状态
2. 用 `ASTMap` 保存 `SubQuestionId -> 条件树`
3. 每次输入后重新 `Evaluate`

它不记录操作历史，不做回放，不管理考试流程。

## 核心数据

### Content

运行时状态表：

```cpp
TMap<FString, float> Content;
```

含义：

```text
ActionId -> Value
```

示例：

```text
RollerHeight -> 2.8
SprayOpen -> 1.0
PumpPressure -> 31.0
```

如果同一个 `ActionId` 重复提交，旧值会被覆盖。

### ASTMap

题目规则表：

```cpp
TMap<FString, UConditionNode*> ASTMap;
```

含义：

```text
SubQuestionId -> ConditionAST
```

示例：

```text
Q_001 -> AND(RollerHeight > 2.5, SprayOpen == 1.0)
Q_002 -> OR(PumpPressure >= 30.0, ValveOpen == 1.0)
```

`ASTMap` 是 `UExamJudgeSubsystem` 上的可编辑属性，可以在蓝图子类或实例默认值中配置。

## 条件树

`UConditionNode` 支持三种节点：

- `Single`：比较 `Content` 中某个 `ActionId` 的当前值
- `AND`：所有子节点都为 true
- `OR`：任意子节点为 true

`Single` 条件包含：

```cpp
FString ActionId;
EExamJudgeCompareType CompareType;
float TargetValue;
```

支持的比较符：

```text
>
<
==
>=
<=
```

示例条件树：

```text
AND
  Single: RollerHeight > 2.5
  Single: SprayOpen == 1.0
```

## 唯一运行流程

### 1. 配置 ASTMap

在 `UExamJudgeSubsystem` 的 `ASTMap` 中配置：

```text
Key: Q_001
Value:
  AND
    Single: RollerHeight > 2.5
    Single: SprayOpen == 1.0
```

布尔状态建议用 float 表示：

```text
false -> 0.0
true  -> 1.0
```

### 2. 切换当前小题

考试流程切题时调用：

```cpp
JudgeSubsystem->SetCurrentSubQuestion(TEXT("Q_001"));
```

该接口只做两件事：

```cpp
CurrentSubQuestionId = SubQuestionId;
Content.Empty();
```

### 3. 提交操作并判定

用户每进行一次操作，调用：

```cpp
const bool bCompleted = JudgeSubsystem->SubmitAction(
    TEXT("RollerHeight"),
    2.8f
);
```

再提交另一个状态：

```cpp
const bool bCompleted = JudgeSubsystem->SubmitAction(
    TEXT("SprayOpen"),
    1.0f
);
```

`SubmitAction` 内部只做：

```cpp
Content.Add(ActionId, Value);
return Evaluate();
```

返回值为 `true` 表示当前小题完成。

## C++ 示例

```cpp
UExamJudgeSubsystem* JudgeSubsystem =
    GameInstance->GetSubsystem<UExamJudgeSubsystem>();

JudgeSubsystem->SetCurrentSubQuestion(TEXT("Q_001"));

JudgeSubsystem->SubmitAction(TEXT("RollerHeight"), 2.8f);
const bool bCompleted = JudgeSubsystem->SubmitAction(TEXT("SprayOpen"), 1.0f);
```

## 蓝图使用

公开接口只有两个：

- `Set Current Sub Question`
- `Submit Action`

蓝图流程：

```text
切题
  -> Set Current Sub Question("Q_001")

用户操作
  -> Submit Action("RollerHeight", 2.8)
  -> Submit Action("SprayOpen", 1.0)
  -> 返回 true 表示完成
```

## 注意事项

- `SubmitAction` 不传 `SubQuestionId`
- 当前小题只由 `SetCurrentSubQuestion` 维护
- 切题会清空 `Content`
- 系统不保存历史记录
- 所有状态值都是 `float`
- `ActionId` 必须和 AST 中的 `ActionId` 完全一致
- 当前 `SubQuestionId` 在 `ASTMap` 中找不到时返回 `false`
- 空的 `AND` / `OR` 节点返回 `false`

