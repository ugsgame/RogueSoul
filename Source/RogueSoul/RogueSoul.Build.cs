// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RogueSoul : ModuleRules
{
	public RogueSoul(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "RogueSoul.h";

        PrivateIncludePaths.AddRange(
			new string[] {
				"RogueSoul/",
				"RogueSoul/AI",
				"RogueSoul/Bots",
				"RogueSoul/Effects",
				"RogueSoul/Pickups",
				"RogueSoul/Player",
				"RogueSoul/Sound",
				"RogueSoul/Weapons",
				"RogueSoul/Online",
				"RogueSoul/UI",
				"RogueSoul/UI/Menu",
				"RogueSoul/UI/Style",
				"RogueSoul/UI/Widgets",
            }
		);

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"AssetRegistry",
                "AIModule",
				"GameplayTasks",
                "Landscape",
                "PhysXVehicles",
                "NavigationSystem",
            }
		);

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"ShooterGameLoadingScreen",
				"Json"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"OnlineSubsystemNull",
				"NetworkReplayStreaming",
				"NullNetworkReplayStreaming",
				"HttpNetworkReplayStreaming"
			}
		);

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"NetworkReplayStreaming"
			}
		);
	}
}
