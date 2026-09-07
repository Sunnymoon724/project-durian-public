// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_Durian : ModuleRules
{
	public Project_Durian(ReadOnlyTargetRules Target) : base(Target)
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
			"Project_Durian",
			"Project_Durian/Variant_Platforming",
			"Project_Durian/Variant_Platforming/Animation",
			"Project_Durian/Variant_Combat",
			"Project_Durian/Variant_Combat/AI",
			"Project_Durian/Variant_Combat/Animation",
			"Project_Durian/Variant_Combat/Gameplay",
			"Project_Durian/Variant_Combat/Interfaces",
			"Project_Durian/Variant_Combat/UI",
			"Project_Durian/Variant_SideScrolling",
			"Project_Durian/Variant_SideScrolling/AI",
			"Project_Durian/Variant_SideScrolling/Gameplay",
			"Project_Durian/Variant_SideScrolling/Interfaces",
			"Project_Durian/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
