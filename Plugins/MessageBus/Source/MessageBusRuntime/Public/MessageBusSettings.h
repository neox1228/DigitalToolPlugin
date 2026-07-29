#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MessageBusSettings.generated.h"

class UMessageSchemaAsset;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="MessageBus"))
class MESSAGEBUSRUNTIME_API UMessageBusSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 运行时默认使用的消息 Schema。GameInstance 初始化 MessageBusSubsystem 时会自动加载它。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Schema")
	TSoftObjectPtr<UMessageSchemaAsset> DefaultSchema;

	// 为 true 时，发送或绑定不存在于 Schema 中的 Topic 会被拒绝并输出日志。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Validation")
	bool bRejectUnknownTopics = true;

	// 为 true 时，Payload 内部 USTRUCT 类型必须和 Schema 配置完全一致。
	// 当前 UScriptStruct 没有类似 UClass 的继承匹配语义，因此建议保持 true。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Validation")
	bool bRequireExactPayloadType = true;

	virtual FName GetCategoryName() const override { return TEXT("Project"); }
};
