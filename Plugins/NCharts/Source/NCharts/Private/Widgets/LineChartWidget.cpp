#include "Widgets/LineChartWidget.h"

#include "Core/NChartRegistry.h"
#include "Features/AxisX/AxisXProxy.h"
#include "Features/AxisY/AxisYProxy.h"
#include "Features/LineSeries/LineSeriesProxy.h"
#include "Features/Tooltip/TooltipProxy.h"
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
	AxisXProxy.Reset();
	AxisYProxy.Reset();
	LineProxy.Reset();
	TooltipProxy.Reset();

	SyncFeatureEntries();

	const FNChartRegistry& Registry = FNChartRegistry::Get();
	const TSharedRef<SOverlay> RootOverlay = SNew(SOverlay);
	TSharedPtr<SWidget> DeferredTooltipWidget;

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
		else if (Entry.FeatureType == EChartFeatureType::Tooltip)
		{
			TooltipProxy = StaticCastSharedPtr<FTooltipProxy>(BaseProxy);
		}

		TSharedPtr<SWidget> FeatureWidget = Registry.CreateWidget(Descriptor.FeatureName, BaseProxy.ToSharedRef());
		if (!FeatureWidget.IsValid())
		{
			FeatureWidget = SNullWidget::NullWidget;
		}

		if (Entry.FeatureType == EChartFeatureType::Tooltip)
		{
			DeferredTooltipWidget = FeatureWidget;
			continue;
		}

		RootOverlay->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			FeatureWidget.ToSharedRef()
		];
	}

	if (DeferredTooltipWidget.IsValid())
	{
		RootOverlay->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			DeferredTooltipWidget.ToSharedRef()
		];
	}

	if (TooltipProxy.IsValid())
	{
		TooltipProxy->SetTargetLineProxy(LineProxy);
	}

	return RootOverlay;
}

void ULineChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	AxisXProxy.Reset();
	AxisYProxy.Reset();
	LineProxy.Reset();
	TooltipProxy.Reset();
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

		if (Entry.FeatureType == EChartFeatureType::AxisX && AxisXProxy.IsValid())
		{
			Entry.Config->ApplyToProxy(AxisXProxy.ToSharedRef());
		}
		else if (Entry.FeatureType == EChartFeatureType::AxisY && AxisYProxy.IsValid())
		{
			Entry.Config->ApplyToProxy(AxisYProxy.ToSharedRef());
		}
		else if (Entry.FeatureType == EChartFeatureType::LineSeries && LineProxy.IsValid())
		{
			Entry.Config->ApplyToProxy(LineProxy.ToSharedRef());
		}
		else if (Entry.FeatureType == EChartFeatureType::Tooltip && TooltipProxy.IsValid())
		{
			Entry.Config->ApplyToProxy(TooltipProxy.ToSharedRef());
		}
	}

	if (TooltipProxy.IsValid())
	{
		TooltipProxy->SetTargetLineProxy(LineProxy);
	}
}
