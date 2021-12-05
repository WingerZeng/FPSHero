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
#include "Net/UnrealNetwork.h"
AFPSHeroWeaponBase::AFPSHeroWeaponBase()
{
	SetReplicates(true);

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
	SetOwner(MyOwner);
	SlotInOwner = WeaponSlot;
}

void AFPSHeroWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSHeroWeaponBase, SlotInOwner);

	DOREPLIFETIME(AFPSHeroWeaponBase, bIsWeaponActive);

	DOREPLIFETIME(AFPSHeroWeaponBase, Owner);

	DOREPLIFETIME(AFPSHeroWeaponBase, Mode)
}

void AFPSHeroWeaponBase::SwitchFireMode()
{
	if (!IsFireModeLocked && GetOwnerCharacter()->GetLocalRole() == ROLE_Authority) {
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
	SetOwnerCharacter(nullptr);
	SetOwner(nullptr);
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
	if (GetOwnerCharacter()->GetLocalRole() == ROLE_Authority) {
		bIsWeaponActive = bActive;
		OnActiveStateChanged();
	}
}

void AFPSHeroWeaponBase::OnActiveStateChanged()
{
	MeshComp->SetVisibility(bIsWeaponActive, true);
}
