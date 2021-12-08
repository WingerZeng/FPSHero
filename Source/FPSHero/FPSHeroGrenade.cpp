// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSHeroGrenade.h"

#include "DrawDebugHelpers.h"
#include "FPSHero.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "FPSHeroCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
//#TEST
#include "DrawDebugHelpers.h"

AFPSHeroGrenade::AFPSHeroGrenade()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->SetCollisionResponseToChannel(TRACECHANNEL_WEAPON, ECR_Ignore);
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSHeroGrenade::OnHit);
	RootComponent = CollisionComp;
	MeshComp->SetupAttachment(CollisionComp);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->bSimulationEnabled = false;
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;

	ShouldHoldByTwoHands = false;	

	ThrowSectionName = "Throw";
	ThrowOutNotifyName = "ThrowOut";
	ThrowReadyNotifyName = "ThrowReady";

	this->PrimaryActorTick.bCanEverTick = true;
}

void AFPSHeroGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Saved For Hit Explosion
}

void AFPSHeroGrenade::EndFire(EFireEndReason Reason)
{
	// Do Nothing if not ready
	if(GetOwnerCharacter() && GetOwnerCharacter()->GetLocalRole() == ROLE_Authority && !bThrown){
		if (bReadyToThrow && (Reason == EFireEndReason::MOUSE_REALEASE || Reason == EFireEndReason::REMOVE_WEAPON)) {
			if (GetOwnerCharacter()->GetLocalRole() == ROLE_Authority)
				StartReleaseMulticast();
		}
		else {
			StopFireMontage();
		}
	}
	bReadyToThrow = false;
}

void AFPSHeroGrenade::Fire()
{
	StartPrepare();
}

void AFPSHeroGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSHeroGrenade, bThrown);
}

void AFPSHeroGrenade::DetachFromCharacter()
{
	CollisionComp->SetWorldTransform(MeshComp->GetComponentTransform());
	this->MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	this->MeshComp->AttachToComponent(this->CollisionComp, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFPSHeroGrenade::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFPSHeroGrenade::ThrowOut()
{
	if (GetOwnerCharacter()->GetLocalRole() == ROLE_Authority) {
		SetReplicateMovement(true);
		bThrown = true;
		SetupThrownMovement();
		GetWorld()->GetTimerManager().SetTimer(ExplodeTimer, this, &AFPSHeroGrenade::Explode, AutoExplodeTime, false, AutoExplodeTime);
	}
}

void AFPSHeroGrenade::SetupThrownMovement_Implementation()
{
	DetachFromCharacter();
	ProjectileMovement->bSimulationEnabled = true;
	FVector EyeLoc;
	FRotator EyeRot;
	GetOwnerCharacter()->GetActorEyesViewPoint(EyeLoc, EyeRot);
	FVector Front = EyeRot.Vector();
	Front *= LauchVelocity;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->IgnoreActorWhenMoving(GetOwnerCharacter(), true);
	ProjectileMovement->SetVelocityInLocalSpace(CollisionComp->GetComponentTransform().InverseTransformVector(Front));
}

void AFPSHeroGrenade::OnActiveStateChanged()
{
	Super::OnActiveStateChanged();
	if (!bIsWeaponActive && !bThrown) {
		bReadyToThrow = false;
		GetOwnerCharacter()->GetAnimInstance()->Montage_Stop(0.4, ThrowMontage);
	}
}

void AFPSHeroGrenade::StartReleaseMulticast_Implementation()
{
	StartRelease();
}

void AFPSHeroGrenade::Explode()
{
	if (GetOwnerCharacter()->GetLocalRole() == ROLE_Authority) {
		PlayExplodeEffect();
		if (DamageType)
			UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BaseDamage, 0, MeshComp->GetComponentLocation(), InnerRadius, OuterRadius, DamageFallOff, DamageType, TArray<AActor*>(), GetOwnerCharacter(), GetOwnerCharacter()->GetController(), ECC_MAX);
		bool ret = Destroy();
	}
}

void AFPSHeroGrenade::PlayExplodeEffect_Implementation()
{
	if (ExplodeEffect)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeEffect, CollisionComp->GetComponentTransform());
	if (ExplodeSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, CollisionComp->GetComponentLocation());
}

void AFPSHeroGrenade::StopFireMontage_Implementation()
{
	GetOwnerCharacter()->GetAnimInstance()->Montage_Stop(0.4, ThrowMontage);
}

void AFPSHeroGrenade::ReadyToThrow()
{
	bReadyToThrow = true;
}
