// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSHeroCharacter.h"
#include "MobaUpgradeEntryBase.h"
#include "MobaPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class FPSHERO_API AMobaPlayerCharacter : public AFPSHeroCharacter
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void ApplyEntry(TSubclassOf<UMobaUpgradeEntryBase> EntryType);

	UFUNCTION(NetMulticast, Reliable)
	void ApplyEntryMulticast(TSubclassOf<UMobaUpgradeEntryBase> EntryType);
	
};
