#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "MessageBusTypes.h"
#include "MessageBusBlueprintLibrary.generated.h"

class UMessageBusSubsystem;
class UMessageSchemaAsset;

UCLASS()
class MESSAGEBUSRUNTIME_API UMessageBusBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 获取当前 World 所属 GameInstance 上的全局消息路由器。
	UFUNCTION(BlueprintPure, Category="MessageBus", meta=(WorldContext="WorldContextObject"))
	static UMessageBusSubsystem* GetMessageBus(UObject* WorldContextObject);

	// 运行时替换当前 Schema，并重建 Topic 查找表。
	// 注意：切换 Schema 时子系统会清空已有监听，避免旧 Topic/旧类型继续生效。
	UFUNCTION(BlueprintCallable, Category="MessageBus|Schema", meta=(WorldContext="WorldContextObject"))
	static void SetSchema(UObject* WorldContextObject, UMessageSchemaAsset* Schema);

	// 蓝图 Topic 参数下拉菜单的数据源。
	// 返回值来自 Project Settings -> MessageBus -> DefaultSchema 中配置的 MessageSchemaAsset。
	UFUNCTION(BlueprintPure, Category="MessageBus|Schema")
	static TArray<FName> GetMessageTopicOptions();

	// 蓝图发送节点：Payload 是“任意 Struct”通配引脚。
	// 蓝图使用时可直接把 Make 你的消息结构体 接到 Payload，不需要手动 MakeInstancedStruct。
	// 内部会自动包装成 FInstancedStruct，然后调用 UMessageBusSubsystem::SendMessage 做 Schema 校验和广播。
	UFUNCTION(BlueprintCallable, CustomThunk, Category="MessageBus", meta=(WorldContext="WorldContextObject", CustomStructureParam="Payload", DisplayName="Send Message"))
	static bool SendMessage(UObject* WorldContextObject, UPARAM(meta=(GetOptions="GetMessageTopicOptions")) FName Topic, const int32& Payload);
	DECLARE_FUNCTION(execSendMessage);

	// 订阅指定 Topic。
	// Topic 从 Schema 下拉选择；Callback 收到的是通用 FBusMessage（其中 Payload 为 FInstancedStruct）。
	// 如果后续需要精准解绑这一次订阅，请保存 OutHandle。
	UFUNCTION(BlueprintCallable, Category="MessageBus", meta=(WorldContext="WorldContextObject"))
	static bool BindMessage(UObject* WorldContextObject, UObject* Listener, UPARAM(meta=(GetOptions="GetMessageTopicOptions")) FName Topic, FOnBusMessage Callback, FMessageBusBindingHandle& OutHandle);

	// 蓝图便利拆包节点：从 BindMessage 回调收到的 FBusMessage 中取出具体 Struct。
	// Payload 是“任意 Struct”输出引脚；输出变量类型必须和 Message.Payload 内部类型一致。
	// 返回 true 表示类型匹配且复制成功，false 表示 Message 为空或结构体类型不匹配。
	UFUNCTION(BlueprintCallable, CustomThunk, Category="MessageBus", meta=(CustomStructureParam="Payload", DisplayName="Get Message Payload As Struct"))
	static bool GetMessagePayloadAsStruct(const FBusMessage& Message, int32& Payload);
	DECLARE_FUNCTION(execGetMessagePayloadAsStruct);

	// 通过 BindMessage 返回的 Handle 移除单个订阅。
	UFUNCTION(BlueprintCallable, Category="MessageBus", meta=(WorldContext="WorldContextObject"))
	static bool UnbindMessage(UObject* WorldContextObject, const FMessageBusBindingHandle& Handle);

	// 移除 Listener 拥有的所有 MessageBus 订阅。常用于 Widget Destruct / Actor EndPlay。
	UFUNCTION(BlueprintCallable, Category="MessageBus", meta=(WorldContext="WorldContextObject"))
	static int32 UnbindAll(UObject* WorldContextObject, UObject* Listener);
};
