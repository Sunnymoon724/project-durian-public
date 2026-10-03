// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Durian : ModuleRules
{
	public Durian(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "SlateCore", "ModelViewViewModel", "TitanClimbing", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Niagara" });
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "AnimGraph", "AnimGraphRuntime", "BlueprintGraph", "AssetRegistry" });
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
