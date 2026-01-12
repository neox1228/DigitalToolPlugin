// Copyright 2020 LHM. All Rights Reserved
using System.Collections.Generic;
using UnrealBuildTool;
using System.IO;
using System;

public class SimpleCharts : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Source/ThirdParty/")); }
    }

    private void stageFiles(String[] FilesToStage)
    {
        foreach (var File in FilesToStage)
        {
            RuntimeDependencies.Add(File);
        }
    }

    public SimpleCharts(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Private"),
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "Json",
				// ... add other public dependencies that you statically link with here ...
			}
			);

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
             new string[]
             {
                "ChartWebBrowser",
             }
            );
        }
        else
        {
            PrivateDependencyModuleNames.AddRange(
               new string[]
               {
                    "WebBrowser",
               }
           );
        }


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

          //Add our runtime dependencies                                                     html
          var filesToStage = Directory.GetFiles(Path.Combine(ThirdPartyPath, ""), "*", SearchOption.AllDirectories);
          stageFiles(filesToStage);

    }
}
