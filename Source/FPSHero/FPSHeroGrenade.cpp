// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSHeroGrenade.h"
#include "FPSHero.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "FPSHeroCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

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
	CollisionComp->SetupAttachment(RootComponent);
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
	//#TEST
	//GEngine->AddOnScreenDebugMessage(-1, 20000, FColor::Black, GetDebugName(OtherActor));
	// Only add impulse and destroy projectile if we hit a physics
	//if (Cast<AFPSHeroCharacter>(OtherActor) != Owner && (OtherActor != NULL) && (OtherActor != this))
	//{
	//	//#TEST
	//	GEngine->AddOnScreenDebugMessage(-1, 20000, FColor::Black, "111111");
	//	Explode();
	//}
}

void AFPSHeroGrenade::EndFire(EFireEndReason Reason)
{
	// Do Nothing if not ready
	if (bReadyToThrow && Reason == EFireEndReason::MOUSE_REALEASE) {
		StartRelease();
	}
	else {
		Owner->GetAnimInstance()->Montage_Stop(0.4, ThrowMontage);
	}
	bReadyToThrow = false;
}

void AFPSHeroGrenade::Fire()
{
	StartPrepare();
}

void AFPSHeroGrenade::SetWeaponActive_Implementation(bool bWeaponActive)
{
	Super::SetWeaponActive_Implementation(bWeaponActive);
	if (!bWeaponActive) {
		bReadyToThrow = false;
		Owner->GetAnimInstance()->Montage_Stop(0.4, ThrowMontage);
	}
}

void AFPSHeroGrenade::ThrowOut()
{
	this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ProjectileMovement->bSimulationEnabled = true;
	FVector EyeLoc;
	FRotator EyeRot;
	Owner->GetActorEyesViewPoint(EyeLoc, EyeRot);
	FVector Front = EyeRot.Vector();
	Front *= LauchVelocity;
	ProjectileMovement->SetVelocityInLocalSpace(RootComponent->GetComponentTransform().InverseTransformVector(Front));
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetGenerateOverlapEvents(true);
	GetWorld()->GetTimerManager().SetTimer(ExplodeTimer, this, &AFPSHeroGrenade::Explode, AutoExplodeTime, false, AutoExplodeTime);
}

void AFPSHeroGrenade::Explode()
{
	if(DamageType)
		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BaseDamage, 0, MeshComp->GetComponentLocation(), InnerRadius, OuterRadius, DamageFallOff, DamageType, TArray<AActor*>(), Owner, Owner->GetController(), ECC_MAX);
	if(ExplodeEffect)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeEffect, MeshComp->GetComponentTransform());
	if(ExplodeSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, MeshComp->GetComponentLocation());
	Destroy();
}

void AFPSHeroGrenade::ReadyToThrow()
{
	bReadyToThrow = true;
}
