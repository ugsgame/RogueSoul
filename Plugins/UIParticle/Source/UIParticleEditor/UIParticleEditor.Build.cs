// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

using UnrealBuildTool;

public class UIParticleEditor : ModuleRules
{
    public UIParticleEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = "Private/UIParticleEditorPrivatePCH.h";

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                        "AssetTools",
                        "MainFrame",
                        "Media",
                        "WorkspaceMenuStructure",
                // ... add any modules that your module loads dynamically here ...
            }
        );

        PublicIncludePaths.AddRange(
			new string[] {
				//"UIParticleEditor/Public",
				// ... add public include paths required here ...
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {

                "UIParticleEditor/Private",
                "UIParticleEditor/Private/AssetTools",
                "UIParticleEditor/Private/Customizations",
                "UIParticleEditor/Private/Factories",
                "UIParticleEditor/Private/Models",
                "UIParticleEditor/Private/Shared",
                "UIParticleEditor/Private/Toolkits",
                //"UIParticleEditor/Private/Widgets",
				// ... add other private include paths required here ...
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "UnrealEd",
                "PropertyEditor",
                "UIParticle"
				// ... add other public dependencies that you statically link with here ...
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "ContentBrowser",
                "PropertyEditor",
                "UMG",
                "EditorStyle",
                "UIParticle",
                "RenderCore",
				// ... add private dependencies that you statically link with here ...	
			}
		);

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "Media",
                "UnrealEd",
                "WorkspaceMenuStructure",
            }
        );

    }
}
