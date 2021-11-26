#include "FPSHeroWeaponBase.h"
#include "FPSHero.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "ButtonBoxComponent.h"
#include "FPSHeroCharacter.h"
#include "FPSHeroRecoilBase.h"
AFPSHeroWeaponBase::AFPSHeroWeaponBase()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	this->SetRootComponent(Root);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->bCastDynamicShadow = false;
	MeshComp->CastShadow = false;
	MeshComp->SetupAttachment(Root);
	MeshComp->SetSimulatePhysics(false);
	LeftHandSocketName = "LeftHandSocket";
	Mode = FireMode::Single;
	IsFireModeLocked = false;
	ShouldHoldByTwoHands = true;
}

void AFPSHeroWeaponBase::SetOwnerCharacter(AFPSHeroCharacter* MyOwner, EWeaponSlot WeaponSlot)
{
	this->Owner = MyOwner;
	SlotInOwner = WeaponSlot;
}

void AFPSHeroWeaponBase::SwitchFireMode()
{
	if (!IsFireModeLocked) {
		switch (Mode)
		{
		case FireMode::Auto:
			Mode = FireMode::Single;
			break;
		case FireMode::Single:
			Mode = FireMode::Auto;
			break;
		default:
			break;
		}
	}
	return;
}

void AFPSHeroWeaponBase::Throw()
{
	// #TODO2 Weapon should be thrown to ground in the future
	Destroy();
}

FVector AFPSHeroWeaponBase::GetLeftHandSocketPosition()
{
	return MeshComp->GetSocketLocation(LeftHandSocketName);
}

TSubclassOf<UAnimInstance> AFPSHeroWeaponBase::GetAnimClass(EViewMode ViewMode)
{
	if (ViewMode == EViewMode::FIRST_PERSON) {
		return AnimClassFPS;
	}
	else {
		return AnimClassTPS;
	}
}

void AFPSHeroWeaponBase::SetWeaponActive_Implementation(bool bActive)
{
	MeshComp->SetVisibility(bActive, true);
	bIsWeaponActive = bActive;
}
