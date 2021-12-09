// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#define TRACECHANNEL_WEAPON ECC_GameTraceChannel2

#define PHYSMAT_FLESH	SurfaceType1
#define PHYSMAT_FLESHVULNERABLE  SurfaceType2

#define ETEAM_NONE 0
#define ETEAM_1 1
#define ETEAM_2 2
#define ETEAM_NEUTRAL 3

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
	MAX_SLOT = 2,
};

UENUM(BlueprintType)
enum class EFireEndReason : uint8
{
	SWAP_WEAPON = 0,
	MOUSE_REALEASE = 1,
	SWITCH_FIRE_MODE = 2,
	FACE_TO_BACK = 3,
	SWITCH_VIEW_MODE = 4,
	REMOVE_WEAPON = 5,
	DEAD = 6,
};

UENUM(BlueprintType)
enum class ETEAM : uint8
{
	TEAM_NONE = 0,
	TEAM1 = 1,
	TEAM2 = 2,
	TEAM_NEUTRAL = 3
};

#include "CoreMinimal.h"
