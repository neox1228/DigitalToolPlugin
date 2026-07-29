#include "MessageBusBlueprintLibrary.h"

#include "MessageBusSettings.h"
#include "MessageBusSubsystem.h"
#include "MessageSchemaAsset.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Stack.h"

UMessageBusSubsystem* UMessageBusBlueprintLibrary::GetMessageBus(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		return GI->GetSubsystem<UMessageBusSubsystem>();
	}
	return nullptr;
}

void UMessageBusBlueprintLibrary::SetSchema(UObject* WorldContextObject, UMessageSchemaAsset* Schema)
{
	if (UMessageBusSubsystem* Bus = GetMessageBus(WorldContextObject))
	{
		Bus->SetSchema(Schema);
	}
}

TArray<FName> UMessageBusBlueprintLibrary::GetMessageTopicOptions()
{
	TArray<FName> Options;

	const UMessageBusSettings* Settings = GetDefault<UMessageBusSettings>();
	if (!Settings)
	{
		return Options;
	}

	UMessageSchemaAsset* Schema = Settings->DefaultSchema.IsValid() ? Settings->DefaultSchema.Get() : nullptr;
	if (!Schema && !Settings->DefaultSchema.IsNull())
	{
		Schema = Settings->DefaultSchema.LoadSynchronous();
	}

	if (!Schema)
	{
		return Options;
	}

	// Schema 是蓝图可选 Topic 的唯一来源。这里去重是为了避免重复配置污染下拉菜单。
	for (const FMessageSchemaItem& Item : Schema->Topics)
	{
		if (!Item.Topic.IsNone())
		{
			Options.AddUnique(Item.Topic);
		}
	}

	Options.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});
	return Options;
}

bool UMessageBusBlueprintLibrary::SendMessage(UObject* WorldContextObject, FName Topic, const int32& Payload)
{
	// 普通 C++ 调用不会使用这个占位实现；蓝图会进入 execSendMessage。
	return false;
}

DEFINE_FUNCTION(UMessageBusBlueprintLibrary::execSendMessage)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_PROPERTY(FNameProperty, Topic);

	// 读取 CustomStructureParam 的真实 Struct 类型和内存地址。
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* PayloadPtr = Stack.MostRecentPropertyAddress;
	FStructProperty* PayloadProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	bool bResult = false;

	if (PayloadProp && PayloadPtr)
	{
		FInstancedStruct InstancedPayload;
		InstancedPayload.InitializeAs(PayloadProp->Struct, static_cast<const uint8*>(PayloadPtr));

		P_NATIVE_BEGIN;
		if (UMessageBusSubsystem* Bus = GetMessageBus(WorldContextObject))
		{
			bResult = Bus->SendMessage(Topic, InstancedPayload);
		}
		P_NATIVE_END;
	}

	*(bool*)RESULT_PARAM = bResult;
}

bool UMessageBusBlueprintLibrary::BindMessage(UObject* WorldContextObject, UObject* Listener, FName Topic, FOnBusMessage Callback, FMessageBusBindingHandle& OutHandle)
{
	if (UMessageBusSubsystem* Bus = GetMessageBus(WorldContextObject))
	{
		return Bus->BindMessage(Listener, Topic, Callback, OutHandle);
	}
	OutHandle.Reset();
	return false;
}

bool UMessageBusBlueprintLibrary::GetMessagePayloadAsStruct(const FBusMessage& Message, int32& Payload)
{
	// 普通 C++ 调用不会使用这个占位实现；蓝图会进入 execGetMessagePayloadAsStruct。
	return false;
}

DEFINE_FUNCTION(UMessageBusBlueprintLibrary::execGetMessagePayloadAsStruct)
{
	P_GET_STRUCT_REF(FBusMessage, Message);

	// 读取 Payload 输出引脚实际连接/声明的 Struct 类型。
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* PayloadPtr = Stack.MostRecentPropertyAddress;
	FStructProperty* PayloadProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	bool bResult = false;

	if (PayloadProp && PayloadPtr && Message.Payload.IsValid() && Message.Payload.GetScriptStruct() == PayloadProp->Struct)
	{
		P_NATIVE_BEGIN;
		PayloadProp->Struct->CopyScriptStruct(PayloadPtr, Message.Payload.GetMemory());
		bResult = true;
		P_NATIVE_END;
	}

	*(bool*)RESULT_PARAM = bResult;
}

bool UMessageBusBlueprintLibrary::UnbindMessage(UObject* WorldContextObject, const FMessageBusBindingHandle& Handle)
{
	if (UMessageBusSubsystem* Bus = GetMessageBus(WorldContextObject))
	{
		return Bus->UnbindMessage(Handle);
	}
	return false;
}

int32 UMessageBusBlueprintLibrary::UnbindAll(UObject* WorldContextObject, UObject* Listener)
{
	if (UMessageBusSubsystem* Bus = GetMessageBus(WorldContextObject))
	{
		return Bus->UnbindAll(Listener);
	}
	return 0;
}
