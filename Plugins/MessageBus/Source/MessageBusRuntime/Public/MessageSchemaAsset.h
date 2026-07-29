#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MessageSchemaAsset.generated.h"

USTRUCT(BlueprintType)
struct MESSAGEBUSRUNTIME_API FMessageSchemaItem
{
	GENERATED_BODY()

	// 消息通道名。建议使用稳定的点分命名，例如 UI.OpenPanel 或 Support.Height。
	// 蓝图 SendMessage / BindMessage 节点里的 Topic 下拉菜单就来自这里。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema")
	FName Topic = NAME_None;

	// 这个 Topic 允许发送的载荷结构体类型。
	// SendMessage 会校验传入的 FInstancedStruct 内部类型是否与这里配置的 USTRUCT 一致。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema")
	TObjectPtr<UScriptStruct> PayloadStruct = nullptr;

	// 可选分类，仅用于编辑器和人工管理，不参与运行时路由。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema")
	FName Category = NAME_None;

	// 可选说明文字，方便策划、蓝图使用者理解这个 Topic 的语义。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema", meta = (MultiLine = true))
	FString Description;
};

UCLASS(BlueprintType)
class MESSAGEBUSRUNTIME_API UMessageSchemaAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 项目允许使用的全部 MessageBus Topic。
	// 通常在 Project Settings -> MessageBus -> DefaultSchema 中引用这个资产。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schema")
	TArray<FMessageSchemaItem> Topics;
};
