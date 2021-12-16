// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSHeroPlayerStateBase.h"

#include "FPSHeroCharacter.h"
#include "FPSHeroGameMode.h"
#include "FPSHeroGameStateBase.h"
#include "Net/UnrealNetwork.h"

AFPSHeroPlayerStateBase::AFPSHeroPlayerStateBase()
{
	bReplicates = true;
}

int AFPSHeroPlayerStateBase::GetMoney() const
{
	return Money;
}

void AFPSHeroPlayerStateBase::SetMoney(int NewMoney)
{
	if(GetLocalRole() == ENetRole::ROLE_Authority)
	{
		this->Money = NewMoney;
		OnRep_Money();
	}
}

void AFPSHeroPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSHeroPlayerStateBase, Money);
	DOREPLIFETIME(AFPSHeroPlayerStateBase, TeamID);
}

void AFPSHeroPlayerStateBase::SetTeam(int Team)
{
	if(GetLocalRole() == ENetRole::ROLE_Authority)
	{
		TeamID = Team;
		OnRep_TeamID();
	}
}

int AFPSHeroPlayerStateBase::GetTeam() const
{
	return TeamID;
}

APlayerController* AFPSHeroPlayerStateBase::GetPlayerController()
{
	for(auto It = GetWorld()->GetPlayerControllerIterator(); It;++It)
	{
		if(It->IsValid())
		{
			if(this == (*It)->GetPlayerState<AFPSHeroPlayerStateBase>())
			{
				return It->Get();
			}
		}
	}
	return nullptr;
}

void AFPSHeroPlayerStateBase::OnRep_Money()
{
	if(GetPlayerController()){
		if(AFPSHeroHUD* HUD = Cast<AFPSHeroHUD>(GetPlayerController()->GetHUD()))
		{
			HUD->CharacterUpdate();
		}
	}
}

void AFPSHeroPlayerStateBase::OnRep_TeamID()
{
	if(auto Character = Cast<AFPSHeroCharacter>(GetPawn()))
	{
		Character->TeamID = TeamID;
		Character->OnTeamChanged();
	}
}

void AFPSHeroPlayerStateBase::Birth_Implementation()
{
	if(GetLocalRole() == ROLE_Authority && Cast<AFPSHeroGameMode>(GetWorld()->GetAuthGameMode())->IsGameStarted())
	{
		AFPSHeroGameMode* GameMode = Cast<AFPSHeroGameMode>(GetWorld()->GetAuthGameMode());
		if(GameMode)
		{
			APlayerController* Controller = GetPlayerController();
			if(Controller)
			{
				//先删除之前的
				APawn* Pawn = Controller->GetPawn();
				if(Pawn && Pawn->IsValidLowLevel())
				{
					Pawn->DetachFromControllerPendingDestroy();
					Pawn->Destroy();
				}
				do
				{
					FTransform Transform;
					GameMode->GetPlayerBirthTransform(this, Transform);
					GameMode->RestartPlayerAtTransform(Controller, Transform);
					Pawn = Controller->GetPawn();
				}
				while (!Pawn || !Pawn->IsValidLowLevel()); // 考虑到生成位置被阻挡，需要多次尝试
				AFPSHeroCharacter* Character = Cast<AFPSHeroCharacter>(Pawn);
				Character->OnControllerChanged();
			}
		}	
	}
	
}
