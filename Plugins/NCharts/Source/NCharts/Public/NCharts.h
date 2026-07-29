// Copyright NCharts Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FNChartsModule —— NCharts 插件模块入口
 *
 * 当前 Startup/Shutdown 为空实现。
 * 各 Feature 通过各自 *Feature.cpp 中的静态 Registrar 自动注册到 FNChartRegistry，
 * 不依赖模块启动时的显式初始化。
 */
class FNChartsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
