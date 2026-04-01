// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FocusMultiplayerGame : ModuleRules
{
	public FocusMultiplayerGame(ReadOnlyTargetRules Target) : base(Target)
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
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FocusMultiplayerGame",
			"FocusMultiplayerGame/Variant_Platforming",
			"FocusMultiplayerGame/Variant_Combat",
			"FocusMultiplayerGame/Variant_Combat/AI",
			"FocusMultiplayerGame/Variant_SideScrolling",
			"FocusMultiplayerGame/Variant_SideScrolling/Gameplay",
			"FocusMultiplayerGame/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
