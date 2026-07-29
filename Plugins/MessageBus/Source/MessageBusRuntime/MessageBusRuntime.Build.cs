using UnrealBuildTool;

public class MessageBusRuntime : ModuleRules
{
	public MessageBusRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"StructUtils"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects"
			}
		);
	}
}

