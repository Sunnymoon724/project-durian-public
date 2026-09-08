// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Durian : ModuleRules
{
	public Durian(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Durian",
			"Durian/Variant_Platforming",
			"Durian/Variant_Platforming/Animation",
			"Durian/Variant_Combat",
			"Durian/Variant_Combat/AI",
			"Durian/Variant_Combat/Animation",
			"Durian/Variant_Combat/Gameplay",
			"Durian/Variant_Combat/Interfaces",
			"Durian/Variant_Combat/UI",
			"Durian/Variant_SideScrolling",
			"Durian/Variant_SideScrolling/AI",
			"Durian/Variant_SideScrolling/Gameplay",
			"Durian/Variant_SideScrolling/Interfaces",
			"Durian/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
