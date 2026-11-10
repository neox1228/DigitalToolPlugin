#pragma once

#include "CoreMinimal.h"

struct FChartSpec;

/** Spec 的 JSON 读取。文件放在 Content 下，路径相对 Content 目录 */
class NXCHARTS_API FChartSpecJson
{
public:
	/** 读取并解析。失败时 OutError 给出原因，OutSpec 不被修改 */
	static bool LoadFromFile(const FString& FullPath, FChartSpec& OutSpec, FString& OutError);
};