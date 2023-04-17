// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

using UnrealBuildTool;

public class UIParticle : ModuleRules
{
    public UIParticle(ReadOnlyTargetRules Target) : base(Target)
    {
        //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = "Private/UIParticlePrivatePCH.h";

        PublicIncludePaths.AddRange(
			new string[] {
                //"UIParticle/Public",
                //"UIParticle/Public/Asset",
                //"UIParticle/Public/Widget",
                //"UIParticle/Public/Particle",
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"UIParticle/Private",
                "UIParticle/Private/Asset",
                "UIParticle/Private/Widget",
                "UIParticle/Private/Particle",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "InputCore",
                "Engine",
                "Slate",
				"SlateCore",
                "UMG",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
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
	}
}
