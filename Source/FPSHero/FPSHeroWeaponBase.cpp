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
#include "Engine/Engine.h"
AFPSHeroWeaponBase::AFPSHeroWeaponBase()
{
	bReplicates = true;
	SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	this->SetRootComponent(Root);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->bCastDynamicShadow = false;
	MeshComp->CastShadow = false;
	MeshComp->SetupAttachment(Root);
	MeshComp->SetSimulatePhysics(false);

	//MeshCompForAttach = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	//MeshCompForAttach->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//MeshCompForAttach->bCastDynamicShadow = false;
	//MeshCompForAttach->CastShadow = false;
	//MeshCompForAttach->SetupAttachment(Root);
	//MeshCompForAttach->SetSimulatePhysics(false);
	//MeshCompForAttach->SetVisibility(false);

	LeftHandSocketName = "LeftHandSocket";
	Mode = FireMode::Single;
	IsFireModeLocked = false;
	ShouldHoldByTwoHands = true;

	this->PrimaryActorTick.bCanEverTick = true;
}

void AFPSHeroWeaponBase::SetOwnerCharacter(AFPSHeroCharacter* MyOwner, EWeaponSlot WeaponSlot)
{
	this->OwnerCharacter = MyOwner;
	SetOwner(MyOwner);
	SlotInOwner = WeaponSlot;
}

void AFPSHeroWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSHeroWeaponBase, SlotInOwner);

	DOREPLIFETIME(AFPSHeroWeaponBase, bIsWeaponActive);

	DOREPLIFETIME(AFPSHeroWeaponBase, OwnerCharacter);

	DOREPLIFETIME(AFPSHeroWeaponBase, Mode)
	
	DOREPLIFETIME(AFPSHeroWeaponBase, TotalAmmo)
	
	DOREPLIFETIME(AFPSHeroWeaponBase, Ammo)
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
	SetOwnerCharacter(nullptr, EWeaponSlot::MAX_SLOT);
	SetOwner(nullptr);
	Destroy();
}

void AFPSHeroWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

void AFPSHeroWeaponBase::DetachFromCharacterMulticast_Implementation()
{
	DetachFromCharacter();
}

void AFPSHeroWeaponBase::AttachToCharacter(USkeletalMeshComponent* Mesh, FName SocketName)
{
	FAttachmentTransformRules rule(EAttachmentRule::KeepRelative, true);
	this->MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	this->MeshComp->AttachToComponent(Mesh, rule, SocketName);
}

void AFPSHeroWeaponBase::DetachFromCharacter()
{
	FAttachmentTransformRules rule(EAttachmentRule::KeepRelative, true);
	this->MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	this->MeshComp->AttachToComponent(this->RootComponent, rule);
}

void AFPSHeroWeaponBase::OnAmmoUpdate()
{
	if(GetOwnerCharacter()->IsLocallyControlled())
	{
		AFPSHeroHUD* HUD = GetOwnerCharacter()->GetFPSHeroHUD();
		if(HUD)
		{
			HUD->CharacterUpdate();
		}
	}
}

void AFPSHeroWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	//无论如何在初始化时都要调用，防止发生active值相同不同步的情况
	OnActiveStateChanged();
}

void AFPSHeroWeaponBase::SetWeaponActive_Implementation(bool bActive)
{
	bIsWeaponActive = bActive;
	OnRep_IsWeaponActive();
}

void AFPSHeroWeaponBase::OnActiveStateChanged()
{
	MeshComp->SetVisibility(bIsWeaponActive, true);
}

int AFPSHeroWeaponBase::GetAmmo() const
{
	return Ammo;
}

void AFPSHeroWeaponBase::SetAmmo(int NewAmmo)
{
	this->Ammo = FMath::Min(GetTotalAmmo(), NewAmmo);
	if(GetOwnerCharacter() && GetOwnerCharacter()->GetLocalRole() == ENetRole::ROLE_Authority)
		OnRep_Ammo();
}

int AFPSHeroWeaponBase::GetTotalAmmo() const
{
	return TotalAmmo;
}

void AFPSHeroWeaponBase::SetTotalAmmo(int NewTotalAmmo)
{
	this->TotalAmmo = NewTotalAmmo;
	if(GetOwnerCharacter() && GetOwnerCharacter()->GetLocalRole() == ENetRole::ROLE_Authority)
		OnRep_TotalAmmo();
}

EWeaponSlot AFPSHeroWeaponBase::GetSlot()
{
	return EquipSlot;
}

void AFPSHeroWeaponBase::OnRep_IsWeaponActive()
{
	OnActiveStateChanged();
}

void AFPSHeroWeaponBase::OnRep_Ammo()
{
	OnAmmoUpdate();
}

void AFPSHeroWeaponBase::OnRep_TotalAmmo()
{
	OnAmmoUpdate();
}
