// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FPSHeroAIController.generated.h"

/**
 * 
 */
UCLASS()
class FPSHERO_API AFPSHeroAIController : public AAIController
{
	GENERATED_BODY()
public:
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	float AimingSpeed = 10;
};
