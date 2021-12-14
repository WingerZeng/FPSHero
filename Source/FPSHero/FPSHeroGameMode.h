// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FPSHeroPlayerStateBase.h"
#include "GameFramework/GameModeBase.h"
#include "FPSHeroGameMode.generated.h"

class AFPSHeroCharacter;

UCLASS(minimalapi)
class AFPSHeroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSHeroGameMode();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintNativeEvent)
	void CharacterDie(AFPSHeroCharacter* Character, AController* Killer);
	
	virtual void OnCharacterDie(AFPSHeroCharacter* Character, AController* Killer);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GameStart();

	UFUNCTION(BlueprintCallable)
	int GetDeathPunishMoney();

	UFUNCTION(BlueprintImplementableEvent)
	void GetPlayerBirthTransform(AFPSHeroPlayerStateBase* Player, FTransform& Transform);

	UFUNCTION(BlueprintCallable)
	bool IsGameStarted();
	
protected:
	UPROPERTY(EditDefaultsOnly)
	int DeathPunishMoney = 100;

	bool bIsGameStarted = false;
};



