#include "Core/NChartRegistry.h"

#include "Widgets/SWidget.h"

FNChartRegistry& FNChartRegistry::Get()
{
	static FNChartRegistry Registry;
	return Registry;
}

void FNChartRegistry::RegisterFeature(FName FeatureName, FProxyFactory ProxyFactory, FWidgetFactory WidgetFactory)
{
	FNChartFeatureDescriptor Descriptor;
	Descriptor.FeatureName = FeatureName;
	Descriptor.DisplayName = FText::FromName(FeatureName);
	Descriptor.ProxyFactory = MoveTemp(ProxyFactory);
	Descriptor.WidgetFactory = MoveTemp(WidgetFactory);
	RegisterFeature(Descriptor);
}

void FNChartRegistry::RegisterFeature(const FNChartFeatureDescriptor& Descriptor)
{
	if (Descriptor.FeatureName.IsNone() || !Descriptor.ProxyFactory || !Descriptor.WidgetFactory)
	{
		return;
	}

	FFeatureEntry Entry;
	Entry.Type = Descriptor.Type;
	Entry.FeatureName = Descriptor.FeatureName;
	Entry.DisplayName = Descriptor.DisplayName;
	Entry.ConfigClass = Descriptor.ConfigClass;
	Entry.LayerOrder = Descriptor.LayerOrder;
	Entry.ProxyFactory = Descriptor.ProxyFactory;
	Entry.WidgetFactory = Descriptor.WidgetFactory;
	Entry.PostBuildLink = Descriptor.PostBuildLink;

	Features.Add(Descriptor.FeatureName, MoveTemp(Entry));
	FeatureTypeToName.Add(Descriptor.Type, Descriptor.FeatureName);
}

void FNChartRegistry::UnregisterFeature(FName FeatureName)
{
	if (const FFeatureEntry* Entry = Features.Find(FeatureName))
	{
		FeatureTypeToName.Remove(Entry->Type);
	}
	Features.Remove(FeatureName);
}

bool FNChartRegistry::HasFeature(FName FeatureName) const
{
	return Features.Contains(FeatureName);
}

TSharedPtr<INChartProxy> FNChartRegistry::CreateProxy(FName FeatureName) const
{
	if (const FFeatureEntry* Entry = Features.Find(FeatureName))
	{
		if (Entry->ProxyFactory)
		{
			return Entry->ProxyFactory();
		}
	}

	return nullptr;
}

TSharedPtr<SWidget> FNChartRegistry::CreateWidget(FName FeatureName, const TSharedRef<INChartProxy>& Proxy) const
{
	if (const FFeatureEntry* Entry = Features.Find(FeatureName))
	{
		if (Entry->WidgetFactory)
		{
			return Entry->WidgetFactory(Proxy);
		}
	}

	return nullptr;
}

void FNChartRegistry::ApplyPostBuildLinks(const TMap<EChartFeatureType, TSharedPtr<INChartProxy>>& FeatureProxies) const
{
	for (const TPair<FName, FFeatureEntry>& Pair : Features)
	{
		const FFeatureEntry& Entry = Pair.Value;
		if (Entry.PostBuildLink)
		{
			Entry.PostBuildLink(FeatureProxies);
		}
	}
}

TArray<FName> FNChartRegistry::GetFeatureNames() const
{
	TArray<FName> Names;
	Features.GetKeys(Names);
	return Names;
}

TArray<EChartFeatureType> FNChartRegistry::GetFeatureTypes() const
{
	TArray<EChartFeatureType> Types;
	FeatureTypeToName.GetKeys(Types);
	return Types;
}

bool FNChartRegistry::GetFeatureDescriptorByType(EChartFeatureType Type, FNChartFeatureDescriptor& OutDescriptor) const
{
	const FName* FeatureName = FeatureTypeToName.Find(Type);
	if (!FeatureName)
	{
		return false;
	}
	return GetFeatureDescriptorByName(*FeatureName, OutDescriptor);
}

bool FNChartRegistry::GetFeatureDescriptorByName(FName FeatureName, FNChartFeatureDescriptor& OutDescriptor) const
{
	const FFeatureEntry* Entry = Features.Find(FeatureName);
	if (!Entry)
	{
		return false;
	}

	OutDescriptor.Type = Entry->Type;
	OutDescriptor.FeatureName = Entry->FeatureName;
	OutDescriptor.DisplayName = Entry->DisplayName;
	OutDescriptor.ConfigClass = Entry->ConfigClass;
	OutDescriptor.LayerOrder = Entry->LayerOrder;
	OutDescriptor.ProxyFactory = Entry->ProxyFactory;
	OutDescriptor.WidgetFactory = Entry->WidgetFactory;
	OutDescriptor.PostBuildLink = Entry->PostBuildLink;
	return true;
}
