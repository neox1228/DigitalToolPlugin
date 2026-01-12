// Copyright 2020 LHM. All Rights Reserved

#include "ChartWebBrowserModule.h"
#include "WebBrowserLog.h"
#include "WebBrowserSingleton.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#if WITH_CEF3
#	include "CEF3Utils.h"
#endif

DEFINE_LOG_CATEGORY(LogChartWebBrowser);

static FChartWebBrowserSingleton* WebBrowserSingleton = nullptr;

FWebBrowserInitSettings::FWebBrowserInitSettings()
	: ProductVersion(FString::Printf(TEXT("%s/%s UnrealEngine/%s Chrome/59.0.3071.15"), FApp::GetProjectName(), FApp::GetBuildVersion(), *FEngineVersion::Current().ToString()))
{
}

class FChartWebBrowserModule : public IWebBrowserModule
{
private:
	// IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	virtual IChartWebBrowserSingleton* GetSingleton() override;
	virtual bool CustomInitialize(const FWebBrowserInitSettings& WebBrowserInitSettings) override;
};

IMPLEMENT_MODULE( FChartWebBrowserModule, ChartWebBrowser );

void FChartWebBrowserModule::StartupModule()
{
#if WITH_CEF3
	CEF3Utils::LoadCEF3Modules();
#endif
}

void FChartWebBrowserModule::ShutdownModule()
{
	if (WebBrowserSingleton != nullptr)
	{
		delete WebBrowserSingleton;
		WebBrowserSingleton = nullptr;
	}

#if WITH_CEF3
	CEF3Utils::UnloadCEF3Modules();
#endif
}

bool FChartWebBrowserModule::CustomInitialize(const FWebBrowserInitSettings& WebBrowserInitSettings)
{
	if (WebBrowserSingleton == nullptr)
	{
		WebBrowserSingleton = new FChartWebBrowserSingleton(WebBrowserInitSettings);
		return true;
	}
	return false;
}

IChartWebBrowserSingleton* FChartWebBrowserModule::GetSingleton()
{
	if (WebBrowserSingleton == nullptr)
	{
		WebBrowserSingleton = new FChartWebBrowserSingleton(FWebBrowserInitSettings());
	}
	return WebBrowserSingleton;
}
