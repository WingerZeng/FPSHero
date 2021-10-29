#include "FPSHero.h"
#include "FPSHeroWeapon.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
AFPSHeroWeapon::AFPSHeroWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	MussleName = "MussleSocket";

	MeshComp->SetCollisionResponseToChannel(TRACECHANNEL_WEAPON, ECR_Ignore);
	MeshComp->bCastDynamicShadow = false;
	MeshComp->CastShadow = false;

	BodyDamage = 50;
	HeadDamage = 100;
}

void AFPSHeroWeapon::Fire()
{
	AActor* owner = GetOwner();
	if (owner) {
		FVector eyeLoc;
		FRotator eyeDir;
		owner->GetActorEyesViewPoint(eyeLoc, eyeDir);

		FVector traceEnd = eyeLoc + eyeDir.Vector() * 10000;
		FHitResult Hit;
		FCollisionQueryParams para;
		//忽略角色和枪模型
		para.AddIgnoredActor(owner);
		para.AddIgnoredActor(this);
		//使用复杂碰撞来求交
		para.bTraceComplex = true;
		para.bReturnPhysicalMaterial = true;
		if (GetWorld()->LineTraceSingleByChannel(Hit, eyeLoc, traceEnd, TRACECHANNEL_WEAPON, para)) {
			dealHit(Hit);
		}

		PlayFireEffect();
	}
}

void AFPSHeroWeapon::dealHit_Implementation(const FHitResult& Hit)
{
	EPhysicalSurface physType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	UParticleSystem* EffectToPlay;

	switch (physType) {
	case PHYSMAT_FLESH: //default flesh
		EffectToPlay = DefaultFleshEffect;
		UGameplayStatics::ApplyDamage(Hit.GetActor(), BodyDamage, GEngine->GetFirstLocalPlayerController(GetWorld()), this, DamageType);
		break;
	case PHYSMAT_FLESHVULNERABLE: //vulnerable flesh
		EffectToPlay = HeadshotEffect;
		UGameplayStatics::ApplyDamage(Hit.GetActor(), HeadDamage, GEngine->GetFirstLocalPlayerController(GetWorld()), this, DamageType);
		break;
	default:
		EffectToPlay = DefaultImpactEffect;
		break;
	}

	if (EffectToPlay) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EffectToPlay, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
}

void AFPSHeroWeapon::PlayFireEffect()
{
	if (MussleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MussleEffect, MeshComp, MussleName);
	}

	if (ShakeType)
		this->GetInstigatorController<APlayerController>()->ClientPlayCameraShake(ShakeType);
}
