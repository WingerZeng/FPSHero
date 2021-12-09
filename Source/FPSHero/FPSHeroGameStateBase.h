// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FPSHeroGameStateBase.generated.h"

class AFPSHeroCharacter;
/**
 * 
 */
UCLASS()
class FPSHERO_API AFPSHeroGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddMoneyForAllPlayer(int AddMoney);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetMoneyForAllPlayer(int NewMoney);
};
