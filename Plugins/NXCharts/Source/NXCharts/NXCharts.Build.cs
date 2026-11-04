// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NXCharts : ModuleRules
{
	public NXCharts(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// 说明：本插件的 Public 头文件中直接暴露了 UObject/Engine/Slate 类型
		// （ChartPaintContext.h 用到 Slate 绘制类型，ChartTheme.h 用到 UDataAsset），
		// 按 UE 的模块规则，这些依赖必须在 Public 中，否则使用方会编译失败。
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"InputCore"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
