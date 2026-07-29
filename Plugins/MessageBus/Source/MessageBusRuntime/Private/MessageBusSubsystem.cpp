#include "MessageBusSubsystem.h"

#include "MessageBusSettings.h"
#include "MessageSchemaAsset.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

static const TCHAR* LogPrefix = TEXT("[MessageBus]");

void UMessageBusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 子系统随 GameInstance 创建后，自动加载项目设置中的默认 Schema。
	// 这样蓝图不需要手动初始化 MessageBus，就可以直接 Send/Bind。
	const UMessageBusSettings* Settings = GetDefault<UMessageBusSettings>();
	if (Settings && Settings->DefaultSchema.IsValid())
	{
		SetSchema(Settings->DefaultSchema.Get());
	}
	else if (Settings && !Settings->DefaultSchema.IsNull())
	{
		UMessageSchemaAsset* Loaded = Settings->DefaultSchema.LoadSynchronous();
		SetSchema(Loaded);
	}
}

void UMessageBusSubsystem::Deinitialize()
{
	// GameInstance 销毁时释放所有运行时通道和监听记录。
	TopicRuntime.Reset();
	Schema = nullptr;
	Super::Deinitialize();
}

void UMessageBusSubsystem::SetSchema(UMessageSchemaAsset* NewSchema)
{
	Schema = NewSchema;
	RebuildRuntimeFromSchema();
}

bool UMessageBusSubsystem::IsTopicValid(const FName Topic) const
{
	return TopicRuntime.Contains(Topic);
}

UScriptStruct* UMessageBusSubsystem::GetTopicPayloadStruct(const FName Topic) const
{
	if (const FMessageBusTopicRuntime* Rt = TopicRuntime.Find(Topic))
	{
		return Rt->PayloadStruct;
	}
	return nullptr;
}

void UMessageBusSubsystem::RebuildRuntimeFromSchema()
{
	TopicRuntime.Reset();

	if (!Schema)
	{
		return;
	}

	for (const FMessageSchemaItem& Item : Schema->Topics)
	{
		// 非法配置在运行时直接忽略；编辑器模块的 Validator 会负责提示资产错误。
		if (Item.Topic.IsNone() || Item.PayloadStruct == nullptr)
		{
			continue;
		}

		FMessageBusTopicRuntime& Rt = TopicRuntime.FindOrAdd(Item.Topic);
		Rt.PayloadStruct = Item.PayloadStruct;
		// Schema 变更代表协议表变了，旧监听可能已经不再合法，因此这里主动清空监听。
		Rt.Listeners.Reset();
	}
}

bool UMessageBusSubsystem::ValidateTopicAndPayload(const FName Topic, const FInstancedStruct& Payload, UScriptStruct*& OutExpectedStruct) const
{
	OutExpectedStruct = nullptr;

	const UMessageBusSettings* Settings = GetDefault<UMessageBusSettings>();
	const bool bRejectUnknown = Settings ? Settings->bRejectUnknownTopics : true;
	const bool bExact = Settings ? Settings->bRequireExactPayloadType : true;

	// 第一步：Topic 必须能在运行时通道表里找到。
	const FMessageBusTopicRuntime* Rt = TopicRuntime.Find(Topic);
	if (!Rt || !Rt->PayloadStruct)
	{
		if (bRejectUnknown)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s Unknown topic '%s' (no schema mapping)."), LogPrefix, *Topic.ToString());
			return false;
		}
		return true;
	}

	OutExpectedStruct = Rt->PayloadStruct;

	if (!Payload.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Topic '%s' requires payload struct '%s' but payload is unset."), LogPrefix, *Topic.ToString(), *Rt->PayloadStruct->GetName());
		return false;
	}

	// 第二步：FInstancedStruct 内部实际保存的 UScriptStruct 必须与 Schema 匹配。
	const UScriptStruct* Actual = Payload.GetScriptStruct();
	if (!Actual)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Topic '%s' payload has no script struct."), LogPrefix, *Topic.ToString());
		return false;
	}

	if (bExact)
	{
		if (Actual != Rt->PayloadStruct)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s Topic '%s' payload mismatch. Expected '%s', got '%s'."),
				LogPrefix, *Topic.ToString(), *Rt->PayloadStruct->GetName(), *Actual->GetName());
			return false;
		}
	}
	else
	{
		// UScriptStruct 没有像 UClass 那样稳定的蓝图继承匹配语义，所以当前仍按精确匹配处理。
		if (Actual != Rt->PayloadStruct)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s Topic '%s' payload mismatch. Expected '%s', got '%s'."),
				LogPrefix, *Topic.ToString(), *Rt->PayloadStruct->GetName(), *Actual->GetName());
			return false;
		}
	}

	return true;
}

void UMessageBusSubsystem::PruneDeadListeners(const FName Topic)
{
	FMessageBusTopicRuntime* Rt = TopicRuntime.Find(Topic);
	if (!Rt)
	{
		return;
	}

	TArray<FGuid> ToRemove;
	ToRemove.Reserve(Rt->Listeners.Num());

	// Listener 是弱引用。对象销毁后 IsValid 会变成 false，这里把对应绑定从通道中移除。
	for (const TPair<FGuid, FMessageBusListenerEntry>& It : Rt->Listeners)
	{
		if (!It.Value.Listener.IsValid())
		{
			ToRemove.Add(It.Key);
		}
	}

	for (const FGuid& Id : ToRemove)
	{
		Rt->Listeners.Remove(Id);
	}
}

bool UMessageBusSubsystem::SendMessage(const FName Topic, const FInstancedStruct& Payload)
{
	UScriptStruct* Expected = nullptr;
	if (!ValidateTopicAndPayload(Topic, Payload, Expected))
	{
		return false;
	}

	FMessageBusTopicRuntime* Rt = TopicRuntime.Find(Topic);
	if (!Rt)
	{
		// 如果项目设置允许未知 Topic，则这里没有通道可路由，直接认为发送结束。
		return true;
	}

	PruneDeadListeners(Topic);

	if (Rt->Listeners.Num() == 0)
	{
		return true;
	}

	FBusMessage Msg;
	Msg.Topic = Topic;
	Msg.Payload = Payload;

	// 先复制 Id 列表，允许监听者在回调内部调用 UnbindMessage，而不会破坏当前遍历。
	TArray<FGuid> Ids;
	Ids.Reserve(Rt->Listeners.Num());
	for (const TPair<FGuid, FMessageBusListenerEntry>& It : Rt->Listeners)
	{
		Ids.Add(It.Key);
	}

	for (const FGuid& Id : Ids)
	{
		FMessageBusListenerEntry* Entry = Rt->Listeners.Find(Id);
		if (!Entry)
		{
			continue;
		}

		if (!Entry->Listener.IsValid())
		{
			Rt->Listeners.Remove(Id);
			continue;
		}

		if (Entry->Delegate.IsBound())
		{
			Entry->Delegate.Execute(Msg);
		}
	}

	return true;
}

bool UMessageBusSubsystem::BindMessage(UObject* Listener, const FName Topic, FOnBusMessage Callback, FMessageBusBindingHandle& OutHandle)
{
	OutHandle.Reset();

	if (Topic.IsNone())
	{
		return false;
	}

	if (!Callback.IsBound() && Listener == nullptr)
	{
		return false;
	}

	const UMessageBusSettings* Settings = GetDefault<UMessageBusSettings>();
	const bool bRejectUnknown = Settings ? Settings->bRejectUnknownTopics : true;
	if (bRejectUnknown && !TopicRuntime.Contains(Topic))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Cannot bind to unknown topic '%s' (no schema mapping)."), LogPrefix, *Topic.ToString());
		return false;
	}

	UObject* BoundObj = Listener;
	if (!BoundObj)
	{
		// 如果没有显式传 Listener，则尝试使用动态委托绑定的 UObject 作为生命周期宿主。
		BoundObj = Callback.GetUObject();
	}
	if (!BoundObj)
	{
		return false;
	}

	FMessageBusTopicRuntime* Rt = TopicRuntime.Find(Topic);
	if (!Rt)
	{
		// 项目设置允许未知 Topic 时，动态创建一个无类型通道。
		Rt = &TopicRuntime.FindOrAdd(Topic);
	}

	const FGuid Id = FGuid::NewGuid();

	FMessageBusListenerEntry Entry;
	Entry.Listener = BoundObj;
	Entry.Delegate = Callback;

	Rt->Listeners.Add(Id, Entry);

	OutHandle.Id = Id;
	OutHandle.Topic = Topic;
	return true;
}

bool UMessageBusSubsystem::UnbindMessage(const FMessageBusBindingHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	FMessageBusTopicRuntime* Rt = TopicRuntime.Find(Handle.Topic);
	if (!Rt)
	{
		return false;
	}

	return Rt->Listeners.Remove(Handle.Id) > 0;
}

int32 UMessageBusSubsystem::UnbindAll(UObject* Listener)
{
	if (!Listener)
	{
		return 0;
	}

	int32 Removed = 0;
	for (TPair<FName, FMessageBusTopicRuntime>& TopicIt : TopicRuntime)
	{
		TArray<FGuid> ToRemove;
		for (const TPair<FGuid, FMessageBusListenerEntry>& It : TopicIt.Value.Listeners)
		{
			if (It.Value.Listener.Get() == Listener)
			{
				ToRemove.Add(It.Key);
			}
		}
		for (const FGuid& Id : ToRemove)
		{
			Removed += TopicIt.Value.Listeners.Remove(Id) > 0 ? 1 : 0;
		}
	}
	return Removed;
}
