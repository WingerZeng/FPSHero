// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#define TRACECHANNEL_WEAPON ECC_GameTraceChannel2

#define PHYSMAT_FLESH	SurfaceType1
#define PHYSMAT_FLESHVULNERABLE  SurfaceType2

UENUM(BlueprintType)
enum class EViewMode : uint8
{
	FIRST_PERSON = 0,
	THIRD_PERSON = 1,
};

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	RIFLE_SLOT = 0,
	GRENADE_SLOT = 1,
};

UENUM(BlueprintType)
enum class EFireEndReason : uint8
{
	SWAP_WEAPON = 0,
	MOUSE_REALEASE = 1,
	SWITCH_FIRE_MODE = 2,
	FACE_TO_BACK = 3
};

#include "CoreMinimal.h"
