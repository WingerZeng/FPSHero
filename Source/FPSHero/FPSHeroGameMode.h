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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GameEnd();
	
	UFUNCTION(BlueprintCallable)
	int GetDeathPunishMoney();

	UFUNCTION(BlueprintImplementableEvent)
	void GetPlayerBirthTransform(AFPSHeroPlayerStateBase* Player, FTransform& Transform);

	UFUNCTION(BlueprintCallable)
	bool IsGameStarted();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	int DeathPunishMoney = 100;

	bool bIsGameStarted = false;
};



