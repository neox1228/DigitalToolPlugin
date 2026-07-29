#pragma once

#include "Modules/ModuleManager.h"

class FMessageBusEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

