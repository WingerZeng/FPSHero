#include "FPSHeroWeapon.h"
#include "FPSHero.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "FPSHeroCharacter.h"
#include "ButtonBoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
AFPSHeroWeapon::AFPSHeroWeapon()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	this->SetRootComponent(Root);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	MussleName = "MuzzleSocket";

	MeshComp->SetCollisionResponseToChannel(TRACECHANNEL_WEAPON, ECR_Ignore);
	MeshComp->bCastDynamicShadow = false;
	MeshComp->CastShadow = false;
	MeshComp->SetupAttachment(Root);

	BodyDamage = 50;
	HeadDamage = 100;

	this->PrimaryActorTick.bCanEverTick = true;
	ShootIntervalSecond = 1;
	Mode = FireMode::Single;
	FireTimer = 0;
	IsFiring = false;
	SecondsSinceStopFire = 0;

	RestoreSecond = 2;
}

void AFPSHeroWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSHeroWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (ShootIntervalSecond > 0) {
		if (IsFiring && Mode == FireMode::Auto) {
			SecondsSinceStartFire += DeltaSeconds;
			FireTimeForRecoil += DeltaSeconds;
			FireTimer += DeltaSeconds;
			if (FireTimer >= ShootIntervalSecond) {
				FireTimer -= ShootIntervalSecond;
				SingleFire();
			}
			// Pitch increase
			float RecoilPitch;
			GetRecoilPitchWithTime(FireTimeForRecoil, RecoilPitch);
			ApplyRecoilPitch(-RecoilPitch * DeltaSeconds);
		}
		else if(!IsFiring){
			SecondsSinceStopFire += DeltaSeconds;
			if (FireTimeForRecoil > 0) {
				// Pitch retore
				float deltaFireTime = (1 - (RestoreSecond - SecondsSinceStopFire) / RestoreSecond) * FireTimeWhenStop;
				float RecoilPitch;
				GetRecoilPitchWithTime(FireTimeForRecoil, RecoilPitch);
				//ApplyRecoilPitch(RecoilPitch * FGenericPlatformMath::Min(DeltaSeconds, deltaFireTime));
				FireTimeForRecoil = FGenericPlatformMath::Max((RestoreSecond - SecondsSinceStopFire) / RestoreSecond * FireTimeWhenStop, 0.0f);
			}
		}
	}
}

void AFPSHeroWeapon::SetOwner(AFPSHeroCharacter* MyOwner)
{
	this->Owner = MyOwner;
}

void AFPSHeroWeapon::Fire()
{
	IsFiring = true;
	SecondsSinceStopFire = 0;
	SecondsSinceStartFire = 0;
	FireTimeForRecoil += ShootIntervalSecond;
	SingleFire();
}

void AFPSHeroWeapon::EndFire()
{
	IsFiring = false;
	SecondsSinceStopFire = 0;
	SecondsSinceStartFire = 0;
	FireTimeWhenStop = FireTimeForRecoil;
	FireTimer = 0;
}

void AFPSHeroWeapon::SwitchFireMode()
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
		UButtonBoxComponent* box = Cast<UButtonBoxComponent>(Hit.Component);
		if (box)
			box->HitButton(Owner->GetController());
		break;
	}

	if (EffectToPlay) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EffectToPlay, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	if (Owner) {
		Owner->DoFire();
	}
}

void AFPSHeroWeapon::PlayFireEffect()
{
	if (MussleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MussleEffect, MeshComp, MussleName);
	}
	if(FireSound)
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	APlayerController* controller = Cast<APlayerController>(Owner->GetController());
	if (controller) {
		controller->ClientStartCameraShake(CameraShakeType);
	}
}

void AFPSHeroWeapon::SingleFire()
{
	if (Owner) {
		FVector eyeLoc;
		FRotator eyeRot;
		Owner->GetActorEyesViewPoint(eyeLoc, eyeRot);
		FVector eyeDir = eyeRot.Vector();

		/* Deal with Recoil Spread */
		// Apply Directional Spread
		eyeDir.Normalize();
		FVector UpDir(0, 0, 1);
		FVector RightDir = FVector::CrossProduct(eyeDir, UpDir);
		RightDir.Normalize();
		float SpreadUp, SpreadRight;
		GetRecoilDirectionalSpreadWithTime(FireTimeForRecoil, SpreadUp, SpreadRight);
		eyeDir += UpDir * SpreadUp + RightDir * SpreadRight;
		eyeDir.Normalize();
		// Apply Random Spread
		float SpreadScale;
		GetRecoilRandomSpreadWithTime(FireTimeForRecoil, SpreadScale);
		eyeDir = UKismetMathLibrary::RandomUnitVectorInConeInRadians(eyeDir, SpreadScale);

		FVector traceEnd = eyeLoc + eyeDir * 10000;
		FHitResult Hit;
		FCollisionQueryParams para;
		//忽略角色和枪模型
		para.AddIgnoredActor(Owner);
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

void AFPSHeroWeapon::ApplyRecoilPitch(float pitch)
{
	if (!Owner)
		return;
	APlayerController* controller = Cast<APlayerController>(Owner->GetController());
	if (controller) {
		controller->AddPitchInput(pitch);
	}
}
