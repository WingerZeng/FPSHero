// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaPlayerCharacter.h"

void AMobaPlayerCharacter::ApplyEntry(TSubclassOf<UMobaUpgradeEntryBase> EntryType)
{
	UMobaUpgradeEntryBase* Entry = EntryType.GetDefaultObject();
	if(Entry)
		Entry->ApplyToCharacter(this);
	ApplyEntryMulticast(EntryType);
}

void AMobaPlayerCharacter::ApplyEntryMulticast_Implementation(TSubclassOf<UMobaUpgradeEntryBase> EntryType)
{
	UMobaUpgradeEntryBase* Entry = EntryType.GetDefaultObject();
	if(Entry)
		Entry->ApplyToCharacterMulticast(this);
}
