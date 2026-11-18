// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DemoCo : ModuleRules
{
	public DemoCo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GeometryCollectionEngine",
			"FieldSystemEngine",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"DemoCo",
			"DemoCo/Variant_Horror",
			"DemoCo/Variant_Horror/UI",
			"DemoCo/Variant_Shooter",
			"DemoCo/Variant_Shooter/AI",
			"DemoCo/Variant_Shooter/UI",
			"DemoCo/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
