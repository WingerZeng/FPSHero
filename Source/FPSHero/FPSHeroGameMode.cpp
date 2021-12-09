// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSHeroGameMode.h"
#include "FPSHeroHUD.h"
#include "FPSHeroCharacter.h"
#include "FPSHeroPlayerStateBase.h"
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

void AFPSHeroGameMode::CharacterDie_Implementation(AFPSHeroCharacter* Character, AController* Killer)
{
	OnCharacterDie(Character, Killer);
}

void AFPSHeroGameMode::OnCharacterDie(AFPSHeroCharacter* Character, AController* Killer)
{
	if(Character)
	{
		APlayerController* PlayerKiller = Cast<APlayerController>(Killer);
		if(PlayerKiller)
		{
			AFPSHeroPlayerStateBase* PlayerState = PlayerKiller->GetPlayerState<AFPSHeroPlayerStateBase>();
			if(PlayerState)
			{
				if(Character->GetTeam() != PlayerState->GetTeam() || PlayerState->GetTeam() == ETEAM_NONE)
				PlayerState->SetMoney(PlayerState->GetMoney() + Character->GetKillAwardMoney());
			}
		}

		const auto DeadPlayerState = Cast<AFPSHeroPlayerStateBase>(Character->GetPlayerState());
		if(DeadPlayerState)
		{
			DeadPlayerState->SetMoney(FMath::Max(DeadPlayerState->GetMoney() - DeathPunishMoney, 0));
		}
	}
}

void AFPSHeroGameMode::BeginPlay()
{
	Super::BeginPlay();
}

int AFPSHeroGameMode::GetDeathPunishMoney()
{
	return DeathPunishMoney;
}
