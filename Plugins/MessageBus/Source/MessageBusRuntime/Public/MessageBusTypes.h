#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "MessageBusTypes.generated.h"

USTRUCT(BlueprintType)
struct MESSAGEBUSRUNTIME_API FBusMessage
{
	GENERATED_BODY()

	// 本次消息发布到的 Topic。正常情况下它应该存在于当前启用的 MessageSchemaAsset 中。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessageBus")
	FName Topic = NAME_None;

	// 通用消息载荷容器。发送前会根据 Schema 校验它内部实际保存的 USTRUCT 类型。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessageBus")
	FInstancedStruct Payload;
};

USTRUCT(BlueprintType)
struct MESSAGEBUSRUNTIME_API FMessageBusBindingHandle
{
	GENERATED_BODY()

	// BindMessage 成功后由子系统分配的唯一绑定 Id。外部不需要理解它的含义，只用于 UnbindMessage。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MessageBus")
	FGuid Id;

	// 这个绑定 Id 所属的 Topic。解绑时先通过 Topic 找到监听列表，再通过 Id 删除对应监听。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MessageBus")
	FName Topic = NAME_None;

	bool IsValid() const { return Id.IsValid() && !Topic.IsNone(); }
	void Reset() { Id.Invalidate(); Topic = NAME_None; }
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBusMessage, const FBusMessage&, Message);
