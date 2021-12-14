// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSHeroCharacter.h"
#include "MobaBotCharacter.generated.h"

/**
 * 
 */
UCLASS()
class FPSHERO_API AMobaBotCharacter : public AFPSHeroCharacter
{
	GENERATED_BODY()
public:
	AMobaBotCharacter();
	
	virtual void SetViewMode(EViewMode NewViewMode) override;
	
	virtual void OnWeaponUpdate() override;

	virtual USkeletalMeshComponent* GetCurrentMesh() override;

	virtual void Destroyed() override;
protected:
	virtual float InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual float InternalTakeRadialDamage(float Damage, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
