// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaUpgradeEntryBase.h"

void UMobaUpgradeEntryBase::Apply(AFPSHeroCharacter* Character)
{
	ApplyToCharacter(Character);
	ApplyMulticast(Character);
}

void UMobaUpgradeEntryBase::ApplyMulticast_Implementation(AFPSHeroCharacter* Character)
{
	ApplyToCharacterMulticast(Character);
}
