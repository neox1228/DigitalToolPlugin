// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file    NXCharts.h
 * @brief   插件模块入口。
 *
 * 这是一个 Runtime 模块：不依赖任何编辑器模块，打包后可在纯游戏环境运行。
 * 模块本身**不持有任何全局状态**——所有图表状态都挂在 UNXChartWidget 实例上，
 * 因此同一进程里可以同时存在任意多张互不影响的图表。
 */

#pragma once

#include "Modules/ModuleManager.h"

class FNXChartsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
