// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyProject2Target : TargetRules
{
	public MyProject2Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		// UE 5.8：V6 会把 ReturnType/UnreachableCode/Dangling 的 warning level 降成 Off，
		// 与 UnrealEditor 共用的构建环境不一致，UBT 会直接拒绝生成。必须用 V7 对齐新默认值。
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("MyProject2");
	}
}
