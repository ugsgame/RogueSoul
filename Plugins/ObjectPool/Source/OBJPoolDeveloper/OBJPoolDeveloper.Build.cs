using UnrealBuildTool;

public class OBJPoolDeveloper : ModuleRules {
    public OBJPoolDeveloper(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		//
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "Engine",
                "OBJPool",
                "CoreUObject",
				"KismetCompiler",
				"BlueprintGraph"
            }///
        );//
        //
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Slate",
                "Projects",
                "UnrealEd",
                "SlateCore",
                "LevelEditor",
                "PropertyEditor"
            }///
        );//
    }///
}