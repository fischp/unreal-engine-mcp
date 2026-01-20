// Copyright Epic Games, Inc. All Rights Reserved.
// Modified for UE4.27 compatibility

using UnrealBuildTool;

public class UnrealMCP : ModuleRules
{
	public UnrealMCP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDefinitions.Add("UNREALMCP_EXPORTS=1");

		PublicIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(ModuleDirectory, "Public"),
				System.IO.Path.Combine(ModuleDirectory, "Public/Commands")
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(ModuleDirectory, "Private"),
				System.IO.Path.Combine(ModuleDirectory, "Private/Commands")
			}
		);

		// Core dependencies - all available in UE4.27
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Networking",
				"Sockets",
				"HTTP",
				"Json",
				"JsonUtilities",
				"PhysicsCore",
				"UnrealEd",
				"UMG"
			}
		);

		// Private dependencies - UE4.27 compatible
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"Kismet",
				"Projects",
				"AssetRegistry",
				"LevelEditor",
				"PropertyEditor",
				"UMGEditor"
			}
		);

		// Removed UE5-only modules:
		// - EditorSubsystem (UE5+)
		// - EditorScriptingUtilities (UE5+)
		// - ToolMenus (UE4.25+, but API differs)
		// - BlueprintEditorLibrary (UE5+)
		// - BlueprintGraph (simplifying for core features)
		// - KismetCompiler (simplifying for core features)
		// - DeveloperSettings (not needed for core features)

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
