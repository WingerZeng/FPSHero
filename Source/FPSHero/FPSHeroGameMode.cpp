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
	DefaultPawnClass = AFPSHeroCharacter::StaticClass();

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

void AFPSHeroGameMode::GameStart_Implementation()
{
	bIsGameStarted = true;
}

void AFPSHeroGameMode::GameEnd_Implementation()
{
	bIsGameStarted = false;
	for(auto it = GetWorld()->GetControllerIterator(); it; it++)
	{
		AFPSHeroCharacter* Character = Cast<AFPSHeroCharacter>(it->Get()->GetCharacter());
		if(Character)
		{
			Character->SetViewModeServer(EViewMode::THIRD_PERSON);
			Character->UnPossessed();
		}
	}
}

int AFPSHeroGameMode::GetDeathPunishMoney()
{
	return DeathPunishMoney;
}

bool AFPSHeroGameMode::IsGameStarted()
{
	return bIsGameStarted;
}

void AFPSHeroGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueNetId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueNetId, ErrorMessage);

	if(IsGameStarted())
	{
		ErrorMessage = "Game Is Processing!";
	}
}
