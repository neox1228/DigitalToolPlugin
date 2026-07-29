#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "MessageBusTypes.h"
#include "MessageBusSubsystem.generated.h"

class UMessageSchemaAsset;

USTRUCT()
struct MESSAGEBUSRUNTIME_API FMessageBusListenerEntry
{
	GENERATED_BODY()

	// 弱引用监听对象，避免 MessageBus 因为保存监听者而阻止 Actor/Widget/Component 被销毁。
	UPROPERTY()
	TWeakObjectPtr<UObject> Listener;

	// Topic 被发布时执行的蓝图回调。
	UPROPERTY()
	FOnBusMessage Delegate;
};

USTRUCT()
struct MESSAGEBUSRUNTIME_API FMessageBusTopicRuntime
{
	GENERATED_BODY()

	// 这个 Topic 期望的 Payload 结构体类型，从当前 Schema 拷贝而来。
	UPROPERTY()
	TObjectPtr<UScriptStruct> PayloadStruct = nullptr;

	// 绑定 Id -> 监听条目。解绑时先通过 Topic O(1) 找到通道，再通过 Id O(1) 删除监听。
	UPROPERTY()
	TMap<FGuid, FMessageBusListenerEntry> Listeners;
};

UCLASS()
class MESSAGEBUSRUNTIME_API UMessageBusSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Schema 管理：替换当前 Schema 并重建运行时 Topic 映射。
	UFUNCTION(BlueprintCallable, Category="MessageBus|Schema")
	void SetSchema(UMessageSchemaAsset* NewSchema);

	UFUNCTION(BlueprintPure, Category="MessageBus|Schema")
	UMessageSchemaAsset* GetSchema() const { return Schema; }

	// 消息发送：向绑定到 Topic 的所有有效监听者广播 Payload。
	UFUNCTION(BlueprintCallable, Category="MessageBus")
	bool SendMessage(const FName Topic, const FInstancedStruct& Payload);

	// 消息订阅：为 Topic 注册回调。Listener 以弱引用保存，对象销毁后会自动跳过并清理。
	UFUNCTION(BlueprintCallable, Category="MessageBus")
	bool BindMessage(UObject* Listener, const FName Topic, FOnBusMessage Callback, FMessageBusBindingHandle& OutHandle);

	// 删除 Handle 指向的单个监听。
	UFUNCTION(BlueprintCallable, Category="MessageBus")
	bool UnbindMessage(const FMessageBusBindingHandle& Handle);

	// 删除 Listener 拥有的所有监听。
	UFUNCTION(BlueprintCallable, Category="MessageBus")
	int32 UnbindAll(UObject* Listener);

	// Validation helpers
	UFUNCTION(BlueprintPure, Category="MessageBus|Schema")
	bool IsTopicValid(const FName Topic) const;

	UFUNCTION(BlueprintPure, Category="MessageBus|Schema")
	UScriptStruct* GetTopicPayloadStruct(const FName Topic) const;

private:
	// 当前生效的 Schema。通常来自 Project Settings -> MessageBus -> DefaultSchema。
	UPROPERTY()
	TObjectPtr<UMessageSchemaAsset> Schema = nullptr;

	// 运行时快速查找表：Topic -> Payload 类型 + 监听者列表。
	UPROPERTY()
	TMap<FName, FMessageBusTopicRuntime> TopicRuntime;

	// 从 Schema 重建 TopicRuntime。
	void RebuildRuntimeFromSchema();

	// 校验 Topic 是否存在，以及 Payload 的结构体类型是否符合 Schema。
	bool ValidateTopicAndPayload(const FName Topic, const FInstancedStruct& Payload, UScriptStruct*& OutExpectedStruct) const;

	// 清理已经被销毁的监听对象，防止无效回调积累。
	void PruneDeadListeners(const FName Topic);
};
