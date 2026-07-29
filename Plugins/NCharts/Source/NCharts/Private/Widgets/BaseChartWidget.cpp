// Copyright NCharts Plugin. All Rights Reserved.

#include "Widgets/BaseChartWidget.h"

#include "Algo/Sort.h"
#include "Core/NChartCartesianScale.h"
#include "Core/NChartCartesianScaleBuilder.h"
#include "Core/NChartRegistry.h"
#include "Engine/Engine.h"
#include "Features/AxisX/AxisXFeatureConfig.h"
#include "Features/AxisY/AxisYFeatureConfig.h"
#include "Features/LineSeries/LineSeriesFeatureConfig.h"
#include "Features/LineSeries/LineSeriesProxy.h"
#include "Features/PointBubbles/PointBubblesFeatureConfig.h"
#include "Features/PointBubbles/PointBubblesProxy.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/Package.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

namespace NChartsWidgetPrivate
{
	bool HasIllegalPrivateArchetype(const UObject* Object)
	{
		if (!Object)
		{
			return false;
		}

		const UObject* Archetype = Object->GetArchetype();
		// 指向其它包里非 Public 的对象（典型：Default__LineChartWidget 上 NewObject 出来的私有 Config）
		return Archetype
			&& Archetype != Object
			&& !Archetype->HasAnyFlags(RF_Public)
			&& Archetype->GetOutermost() != Object->GetOutermost();
	}

	void DetachConfigToTransient(UNChartFeatureConfigBase* Config)
	{
		if (!Config)
		{
			return;
		}

		Config->Rename(
			nullptr,
			GetTransientPackage(),
			REN_DoNotDirty | REN_DontCreateRedirectors | REN_ForceNoResetLoaders | REN_NonTransactional);
	}

	UNChartFeatureConfigBase* CreateOwnedConfig(
		UObject* Owner,
		UClass* ConfigClass,
		UNChartFeatureConfigBase* CopyFrom)
	{
		UNChartFeatureConfigBase* NewConfig = NewObject<UNChartFeatureConfigBase>(
			Owner,
			ConfigClass,
			NAME_None,
			RF_Transactional);

		if (CopyFrom && CopyFrom != NewConfig && CopyFrom->IsA(ConfigClass))
		{
			UEngine::CopyPropertiesForUnrelatedObjects(CopyFrom, NewConfig);
		}

		return NewConfig;
	}
}

UBaseChartWidget::UBaseChartWidget()
{
	// Features 保持为空，由子类 InitializePresetFeatures 或用户在编辑器中配置
}

bool UBaseChartWidget::AddFeature(EChartFeatureType FeatureType)
{
	if (HasFeature(FeatureType))
	{
		return false;
	}

	FNChartFeatureEntry& Entry = Features.AddDefaulted_GetRef();
	Entry.FeatureType = FeatureType;
	SyncFeatureEntries();
	return true;
}

bool UBaseChartWidget::RemoveFeatureAt(int32 Index)
{
	if (!Features.IsValidIndex(Index))
	{
		return false;
	}

	Features.RemoveAt(Index);
	return true;
}

bool UBaseChartWidget::HasFeature(EChartFeatureType FeatureType) const
{
	for (const FNChartFeatureEntry& Entry : Features)
	{
		if (Entry.FeatureType == FeatureType)
		{
			return true;
		}
	}
	return false;
}

UNChartFeatureConfigBase* UBaseChartWidget::GetFeatureConfig(EChartFeatureType FeatureType) const
{
	for (const FNChartFeatureEntry& Entry : Features)
	{
		if (Entry.FeatureType == FeatureType)
		{
			return Entry.Config;
		}
	}
	return nullptr;
}

void UBaseChartWidget::SetFeaturePoints(EChartFeatureType FeatureType, const TArray<FVector2D>& InPoints)
{
	if (FeatureType == EChartFeatureType::LineSeries)
	{
		if (ULineSeriesFeatureConfig* Config = Cast<ULineSeriesFeatureConfig>(GetFeatureConfig(FeatureType)))
		{
			Config->Points = InPoints;
		}

		if (TSharedPtr<FLineSeriesProxy> Proxy = StaticCastSharedPtr<FLineSeriesProxy>(GetLiveFeatureProxy(FeatureType)))
		{
			Proxy->SetPoints(InPoints);
		}
	}
	else if (FeatureType == EChartFeatureType::PointBubbles)
	{
		if (UPointBubblesFeatureConfig* Config = Cast<UPointBubblesFeatureConfig>(GetFeatureConfig(FeatureType)))
		{
			Config->Points = InPoints;
		}

		if (TSharedPtr<FPointBubblesProxy> Proxy = StaticCastSharedPtr<FPointBubblesProxy>(GetLiveFeatureProxy(FeatureType)))
		{
			Proxy->SetPoints(InPoints);
		}
	}

	BuildAndApplyCartesianScale();
}

void UBaseChartWidget::RefreshFeatureProxies()
{
	SyncFeatureEntries();
	ApplyFeatureEntriesToLiveProxies();
}

void UBaseChartWidget::RebuildChart()
{
	ReleaseSlateResources(true);
	InvalidateLayoutAndVolatility();
}

void UBaseChartWidget::InitializePresetFeatures()
{
}

TSharedRef<SWidget> UBaseChartWidget::RebuildWidget()
{
	ResetLiveProxies();
	SyncFeatureEntries();

	const FNChartRegistry& Registry = FNChartRegistry::Get();
	const TSharedRef<SOverlay> RootOverlay = SNew(SOverlay);

	struct FLayeredWidget
	{
		int32 LayerOrder = 0;
		TSharedPtr<SWidget> Widget;
	};
	TArray<FLayeredWidget> LayeredWidgets;

	for (FNChartFeatureEntry& Entry : Features)
	{
		if (!Entry.Config)
		{
			continue;
		}

		FNChartRegistry::FNChartFeatureDescriptor Descriptor;
		if (!Registry.GetFeatureDescriptorByType(Entry.FeatureType, Descriptor))
		{
			continue;
		}

		const TSharedPtr<INChartProxy> BaseProxy = Registry.CreateProxy(Descriptor.FeatureName);
		if (!BaseProxy.IsValid())
		{
			continue;
		}

		Entry.Config->ApplyToProxy(BaseProxy.ToSharedRef());
		LiveFeatureProxies.Add(Entry.FeatureType, BaseProxy);

		TSharedPtr<SWidget> FeatureWidget = Registry.CreateWidget(Descriptor.FeatureName, BaseProxy.ToSharedRef());
		if (!FeatureWidget.IsValid())
		{
			FeatureWidget = SNullWidget::NullWidget;
		}

		FLayeredWidget& Layered = LayeredWidgets.AddDefaulted_GetRef();
		Layered.LayerOrder = Descriptor.LayerOrder;
		Layered.Widget = FeatureWidget;
	}

	Algo::Sort(LayeredWidgets, [](const FLayeredWidget& A, const FLayeredWidget& B)
	{
		return A.LayerOrder < B.LayerOrder;
	});

	for (const FLayeredWidget& Layered : LayeredWidgets)
	{
		if (!Layered.Widget.IsValid())
		{
			continue;
		}

		RootOverlay->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			Layered.Widget.ToSharedRef()
		];
	}

	Registry.ApplyPostBuildLinks(LiveFeatureProxies);
	BuildAndApplyCartesianScale();

	return RootOverlay;
}

void UBaseChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	ResetLiveProxies();
}

void UBaseChartWidget::ResetLiveProxies()
{
	LiveFeatureProxies.Reset();
	LiveCartesianScale.Reset();
}

void UBaseChartWidget::BuildAndApplyCartesianScale()
{
	const UAxisXFeatureConfig* AxisXConfig = Cast<UAxisXFeatureConfig>(GetFeatureConfig(EChartFeatureType::AxisX));
	const UAxisYFeatureConfig* AxisYConfig = Cast<UAxisYFeatureConfig>(GetFeatureConfig(EChartFeatureType::AxisY));

	LiveCartesianScale = FNChartCartesianScaleBuilder::Build(LiveFeatureProxies, AxisXConfig, AxisYConfig);

	for (const TPair<EChartFeatureType, TSharedPtr<INChartProxy>>& Pair : LiveFeatureProxies)
	{
		if (INChartScaleConsumer* Consumer = Pair.Value.IsValid() ? Pair.Value->GetScaleConsumer() : nullptr)
		{
			Consumer->SetCartesianScale(LiveCartesianScale);
		}
	}
}

TSharedPtr<INChartProxy> UBaseChartWidget::GetLiveFeatureProxy(EChartFeatureType FeatureType) const
{
	if (const TSharedPtr<INChartProxy>* Found = LiveFeatureProxies.Find(FeatureType))
	{
		return *Found;
	}
	return nullptr;
}

void UBaseChartWidget::SyncFeatureEntries()
{
	const FNChartRegistry& Registry = FNChartRegistry::Get();

	// CDO 上绝不能挂 Instanced Config：
	// UMG 蓝图模板会把它们当作 Archetype，而 NewObject 子对象默认是 private，
	// 保存时就会报 Illegal reference to private object（Default__LineChartWidget:XxxConfig_0）。
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		for (FNChartFeatureEntry& Entry : Features)
		{
			if (Entry.Config)
			{
				NChartsWidgetPrivate::DetachConfigToTransient(Entry.Config);
				Entry.Config = nullptr;
			}
		}
		return;
	}

	for (FNChartFeatureEntry& Entry : Features)
	{
		FNChartRegistry::FNChartFeatureDescriptor Descriptor;
		if (!Registry.GetFeatureDescriptorByType(Entry.FeatureType, Descriptor) || !Descriptor.ConfigClass)
		{
			continue;
		}

		const bool bWrongType = !Entry.Config || !Entry.Config->IsA(Descriptor.ConfigClass);
		const bool bWrongOuter = Entry.Config && Entry.Config->GetOuter() != this;
		const bool bIllegalArchetype = NChartsWidgetPrivate::HasIllegalPrivateArchetype(Entry.Config);

		if (!bWrongType && !bWrongOuter && !bIllegalArchetype)
		{
			continue;
		}

		UNChartFeatureConfigBase* OldConfig = Entry.Config;
		Entry.Config = NChartsWidgetPrivate::CreateOwnedConfig(
			this,
			Descriptor.ConfigClass,
			(!bWrongType && OldConfig) ? OldConfig : nullptr);

		// 旧 Config 若仍挂在本 Widget 下，挪走以免带着非法 Archetype 一起被序列化
		if (OldConfig && OldConfig != Entry.Config && OldConfig->GetOuter() == this)
		{
			NChartsWidgetPrivate::DetachConfigToTransient(OldConfig);
		}
	}
}

void UBaseChartWidget::ApplyFeatureEntriesToLiveProxies()
{
	for (const FNChartFeatureEntry& Entry : Features)
	{
		if (!Entry.Config)
		{
			continue;
		}

		const TSharedPtr<INChartProxy>* ProxyPtr = LiveFeatureProxies.Find(Entry.FeatureType);
		if (ProxyPtr && ProxyPtr->IsValid())
		{
			Entry.Config->ApplyToProxy(ProxyPtr->ToSharedRef());
		}
	}

	FNChartRegistry::Get().ApplyPostBuildLinks(LiveFeatureProxies);
	BuildAndApplyCartesianScale();
}

void UBaseChartWidget::PostLoad()
{
	Super::PostLoad();
	SyncFeatureEntries();
}

void UBaseChartWidget::PostInitProperties()
{
	Super::PostInitProperties();

	// CDO 与实例均执行：子类预设（如 LineChart 默认 Feature）需在 CDO 上生效以供编辑器默认值展示
	InitializePresetFeatures();
	SyncFeatureEntries();
}

void UBaseChartWidget::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	// 保存前拆掉指向 CDO 私有 Config 的非法 Archetype，并保证 Config Outer 正确
	SyncFeatureEntries();
	Super::PreSave(ObjectSaveContext);
}

#if WITH_EDITOR
void UBaseChartWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SyncFeatureEntries();
	ApplyFeatureEntriesToLiveProxies();
}

void UBaseChartWidget::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	SyncFeatureEntries();
	ApplyFeatureEntriesToLiveProxies();
}
#endif
