// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FPSHeroHUD.generated.h"

UCLASS()
class AFPSHeroHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFPSHeroHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintImplementableEvent)
	void CharacterUpdate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Die(float RebirthInterval);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Rebirth();
	
private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

