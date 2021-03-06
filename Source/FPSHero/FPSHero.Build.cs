// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPSHero : ModuleRules
{
	public FPSHero(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysicsCore", "AIModule", "UMG"});
    }
}
