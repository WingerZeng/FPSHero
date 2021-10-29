// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSHeroGameMode.h"
#include "FPSHeroHUD.h"
#include "FPSHeroCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPSHeroGameMode::AFPSHeroGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHeroHUD::StaticClass();
}
