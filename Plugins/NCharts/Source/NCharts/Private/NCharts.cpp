// Copyright NCharts Plugin. All Rights Reserved.
// NCharts 模块实现

#include "NCharts.h"

void FNChartsModule::StartupModule()
{
	// Feature 注册由各 *Feature.cpp 中的静态 Registrar 自动完成，此处无需额外操作
}

void FNChartsModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FNChartsModule, NCharts)
