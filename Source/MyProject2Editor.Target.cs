// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyProject2EditorTarget : TargetRules
{
	public MyProject2EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		// UE 5.8：见 MyProject2.Target.cs 的说明，V6 会导致 UBT 拒绝生成。
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("MyProject2");
	}
}
