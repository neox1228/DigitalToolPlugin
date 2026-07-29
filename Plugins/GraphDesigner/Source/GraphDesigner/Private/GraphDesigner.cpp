#include "GraphDesigner.h"

DEFINE_LOG_CATEGORY(LogGraphDesigner);

void FGraphDesignerModule::StartupModule()
{
	UE_LOG(LogGraphDesigner, Log, TEXT("GraphDesigner module started"));
}

void FGraphDesignerModule::ShutdownModule()
{
	UE_LOG(LogGraphDesigner, Log, TEXT("GraphDesigner module shutdown"));
}

IMPLEMENT_MODULE(FGraphDesignerModule, GraphDesigner)
