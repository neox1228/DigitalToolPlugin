#pragma once

#include "Modules/ModuleManager.h"

class FGraphDesignerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(LogGraphDesigner, Log, All);
