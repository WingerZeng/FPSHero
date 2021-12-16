// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FPSHero.h"
#include "FPSHeroCharacter.h"
#include "MobaUpgradeEntryBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract, EditInlineNew, Category = "Entry")
class FPSHERO_API UMobaUpgradeEntryBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Detail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EEntryLevel Level;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	int Cost();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ApplyToCharacter(class AFPSHeroCharacter* Character);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ApplyToCharacterMulticast(class AFPSHeroCharacter* Character);
};


