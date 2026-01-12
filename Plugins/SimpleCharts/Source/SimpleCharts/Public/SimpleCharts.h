// Copyright 2020 LHM. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSimpleChartsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FSimpleChartsModule& Get();
private:
	void CopyHtmlFiles();
	static FSimpleChartsModule* Singleton;
};
