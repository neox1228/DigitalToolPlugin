#include "Widgets/LineChartWidget.h"

#include "Algo/Sort.h"
#include "Core/NChartRegistry.h"
#include "Features/AxisX/AxisXProxy.h"
#include "Features/AxisY/AxisYProxy.h"
#include "Features/LineSeries/LineSeriesProxy.h"
#include "Features/LineSeries/LineSeriesFeatureConfig.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"

namespace
{
	ULineSeriesFeatureConfig* FindLineSeriesConfig(TArray<FNChartFeatureEntry>& Features)
	{
		for (FNChartFeatureEntry& Entry : Features)
		{
			if (Entry.FeatureType == EChartFeatureType::LineSeries)
			{
				return Cast<ULineSeriesFeatureConfig>(Entry.Config);
			}
		}

		return nullptr;
	}
}

ULineChartWidget::ULineChartWidget()
{
	Features.AddDefaulted(4);
	Features[0].FeatureType = EChartFeatureType::AxisX;
	Features[1].FeatureType = EChartFeatureType::AxisY;
	Features[2].FeatureType = EChartFeatureType::LineSeries;
	Features[3].FeatureType = EChartFeatureType::Tooltip;
}

TSharedRef<SWidget> ULineChartWidget::RebuildWidget()
{
	LiveFeatureProxies.Reset();
	AxisXProxy.Reset();
	AxisYProxy.Reset();
	LineProxy.Reset();

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

		if (UNChartFeatureConfigBase* Config = Entry.Config)
		{
			Config->ApplyToProxy(BaseProxy.ToSharedRef());
		}

		LiveFeatureProxies.Add(Entry.FeatureType, BaseProxy);

		if (Entry.FeatureType == EChartFeatureType::AxisX)
		{
			AxisXProxy = StaticCastSharedPtr<FAxisXProxy>(BaseProxy);
		}
		else if (Entry.FeatureType == EChartFeatureType::AxisY)
		{
			AxisYProxy = StaticCastSharedPtr<FAxisYProxy>(BaseProxy);
		}
		else if (Entry.FeatureType == EChartFeatureType::LineSeries)
		{
			LineProxy = StaticCastSharedPtr<FLineSeriesProxy>(BaseProxy);
		}

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

	return RootOverlay;
}

void ULineChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	LiveFeatureProxies.Reset();
	AxisXProxy.Reset();
	AxisYProxy.Reset();
	LineProxy.Reset();
}

void ULineChartWidget::SetPoints(const TArray<FVector2D>& InPoints)
{
	if (ULineSeriesFeatureConfig* Config = FindLineSeriesConfig(Features))
	{
		Config->Points = InPoints;
	}

	if (LineProxy.IsValid())
	{
		LineProxy->SetPoints(InPoints);
	}
}

void ULineChartWidget::SetYLimit(float InLimit, bool bEnable)
{
	if (ULineSeriesFeatureConfig* Config = FindLineSeriesConfig(Features))
	{
		Config->YLimit = InLimit;
		Config->bUseLimit = bEnable;
	}

	if (LineProxy.IsValid())
	{
		LineProxy->SetYLimit(InLimit, bEnable);
	}
}

void ULineChartWidget::SetLineColor(const FLinearColor& InColor)
{
	if (ULineSeriesFeatureConfig* Config = FindLineSeriesConfig(Features))
	{
		Config->LineColor = InColor;
	}

	if (LineProxy.IsValid())
	{
		LineProxy->SetLineColor(InColor);
	}
}

void ULineChartWidget::SyncFeatureEntries()
{
	const FNChartRegistry& Registry = FNChartRegistry::Get();

	for (FNChartFeatureEntry& Entry : Features)
	{
		FNChartRegistry::FNChartFeatureDescriptor Descriptor;
		if (!Registry.GetFeatureDescriptorByType(Entry.FeatureType, Descriptor) || !Descriptor.ConfigClass)
		{
			continue;
		}

		if (!Entry.Config || !Entry.Config->IsA(Descriptor.ConfigClass))
		{
			Entry.Config = NewObject<UNChartFeatureConfigBase>(this, Descriptor.ConfigClass, NAME_None, RF_Transactional);
		}
	}
}

void ULineChartWidget::PostLoad()
{
	Super::PostLoad();
	SyncFeatureEntries();
}

void ULineChartWidget::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SyncFeatureEntries();
	}
}

#if WITH_EDITOR
void ULineChartWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SyncFeatureEntries();
	ApplyFeatureEntriesToLiveProxies();
}

void ULineChartWidget::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	SyncFeatureEntries();
	ApplyFeatureEntriesToLiveProxies();
}
#endif

void ULineChartWidget::ApplyFeatureEntriesToLiveProxies()
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
}
