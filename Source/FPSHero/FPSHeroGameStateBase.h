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
	virtual void OnCharacterDie(AFPSHeroCharacter* Character);
};
