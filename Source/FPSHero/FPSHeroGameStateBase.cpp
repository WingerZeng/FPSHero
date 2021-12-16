// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSHeroGameStateBase.h"

#include "FPSHeroGameMode.h"
#include "FPSHeroPlayerStateBase.h"

void AFPSHeroGameStateBase::AddMoneyForAllPlayer(int AddMoney)
{
	if(GetLocalRole() == ENetRole::ROLE_Authority)
	{
		for(const auto& PlayerState: PlayerArray)
		{
			if(PlayerState)
			{
				AFPSHeroPlayerStateBase* FPSHeroPlayerState = Cast<AFPSHeroPlayerStateBase>(PlayerState);
				if(FPSHeroPlayerState)
				{
					FPSHeroPlayerState->SetMoney(FPSHeroPlayerState->GetMoney() + AddMoney);
				}	
			}
		}
	}
}

void AFPSHeroGameStateBase::SetMoneyForAllPlayer(int NewMoney)
{
	if(GetLocalRole() == ENetRole::ROLE_Authority)
	{
		for(const auto& PlayerState: PlayerArray)
		{
			if(PlayerState)
			{
				AFPSHeroPlayerStateBase* FPSHeroPlayerState = Cast<AFPSHeroPlayerStateBase>(PlayerState);
				if(FPSHeroPlayerState)
				{
					FPSHeroPlayerState->SetMoney(NewMoney);
				}	
			}
		}
	}
}

