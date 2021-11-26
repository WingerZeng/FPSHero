#include "FPSHeroWeapon.h"
#include "FPSHero.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "ButtonBoxComponent.h"
#include "FPSHeroCharacter.h"
#include "FPSHeroRecoilBase.h"
AFPSHeroWeapon::AFPSHeroWeapon()
{
	MussleName = "MuzzleSocket";

	BodyDamage = 50;
	HeadDamage = 100;

	this->PrimaryActorTick.bCanEverTick = true;
	ShootIntervalSecond = 1;
	Mode = FireMode::Single;
}

void AFPSHeroWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(!IsFiring){
		// 停止射击后，计算后坐力恢复和准心恢复
		SecondsSinceStopFire += DeltaSeconds;
		// 后坐力恢复
		if (CurrentFiredAmmo > 0 && RecoilInstance) {
			CurrentFiredAmmo = FGenericPlatformMath::Max(0.0f, 
				FGenericPlatformMath::CeilToFloat((RecoilInstance->RecoilRestoreTime - SecondsSinceStopFire) 
					/ RecoilInstance->RecoilRestoreTime * FiredAmmoWhenStop));
		}
		// 准心恢复
		float RestoreRatio = 1;
		if (RecoilInstance)
			RecoilInstance->GetCameraRestoreRatio(SecondsSinceStopFire, RestoreRatio);
		float TargetPicth = PitchOffsetWhenStop * (1 - RestoreRatio);
		float TargetYaw = YawOffsetWhenStop * (1 - RestoreRatio);
		ApplyNewRecoilCameraOffset(TargetPicth, TargetYaw);
	}
}

void AFPSHeroWeapon::Fire()
{
	IsFiring = true;
	if(Mode == FireMode::Auto && ShootIntervalSecond > 0)
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &AFPSHeroWeapon::SingleFire, ShootIntervalSecond, true, ShootIntervalSecond);
	SingleFire();
}

void AFPSHeroWeapon::EndFire(EFireEndReason Reason)
{
	IsFiring = false;
	SecondsSinceStopFire = 0;
	FiredAmmoWhenStop = CurrentFiredAmmo;
	PitchOffsetWhenStop = PitchOffset;
	YawOffsetWhenStop = YawOffset;
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
}

void AFPSHeroWeapon::dealHit_Implementation(const FHitResult& Hit)
{
	EPhysicalSurface physType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	UParticleSystem* EffectToPlay;

	FVector eyeLoc;
	FRotator eyeRot;
	Owner->GetActorEyesViewPoint(eyeLoc, eyeRot);

	switch (physType) {
	case PHYSMAT_FLESH: //default flesh
		EffectToPlay = DefaultFleshEffect;
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), BodyDamage, Hit.ImpactPoint - eyeLoc, Hit, GEngine->GetFirstLocalPlayerController(GetWorld()), this, DamageType);
		break;
	case PHYSMAT_FLESHVULNERABLE: //vulnerable flesh
		EffectToPlay = HeadshotEffect;
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), HeadDamage, Hit.ImpactPoint - eyeLoc, Hit, GEngine->GetFirstLocalPlayerController(GetWorld()), this, DamageType);
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
		Owner->PlayMontage(FireMontage);
	}
}

void AFPSHeroWeapon::PlayFireEffect()
{
	//枪口特效
	if (MussleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MussleEffect, MeshComp, MussleName);
	}
	//音效
	if(FireSound)
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	APlayerController* Controller = Cast<APlayerController>(Owner->GetController());

	if (RecoilInstance) {
		// 镜头抖动
		RecoilInstance->ApplyCameraShake(CurrentFiredAmmo, Controller);
		// 镜头移动
		float RecoilPitch, RecoilYaw;
		RecoilInstance->GetCameraMovement(CurrentFiredAmmo, RecoilPitch, RecoilYaw);
		ApplyNewRecoilCameraOffset(RecoilPitch, RecoilYaw);
	}
}

void AFPSHeroWeapon::SingleFire()
{
	CurrentFiredAmmo++;
	if (Owner) {
		// #TODO 在第三人称下用枪口延伸
		// 得到摄像机的位置和朝向
		FVector eyeLoc;
		FRotator eyeRot;
		Owner->GetActorEyesViewPoint(eyeLoc, eyeRot);
		FVector eyeDir = eyeRot.Vector();

		/* 后坐力扩散 */
		// 方向性扩散
		// 首先得到镜头的Up、Right方向在世界坐标系下的方向向量，再乘上对应方向的扩散系数
		eyeDir.Normalize();
		FVector UpDir(0, 0, 1);
		FVector RightDir = FVector::CrossProduct(eyeDir, UpDir);
		RightDir.Normalize();
		float SpreadUp=0, SpreadRight=0;
		if (RecoilInstance) {
			RecoilInstance->GetDirectionalSpread(CurrentFiredAmmo, SpreadUp, SpreadRight);
		}
		eyeDir += UpDir * SpreadUp + RightDir * SpreadRight;
		eyeDir.Normalize();
		// 随机扩散
		// 根据扩散系数，在圆锥上取随机向量
		float SpreadScale = 0;
		if (RecoilInstance) {
			RecoilInstance->GetRandomSpread(CurrentFiredAmmo, SpreadScale);
		}
		eyeDir = UKismetMathLibrary::RandomUnitVectorInConeInRadians(eyeDir, SpreadScale);

		// 得到光线追踪目标点
		FVector traceEnd = eyeLoc + eyeDir * 10000;
		FHitResult Hit;
		FCollisionQueryParams para;
		// 忽略角色和枪模型
		para.AddIgnoredActor(Owner);
		para.AddIgnoredActor(this);
		// 使用复杂碰撞来求交
		para.bTraceComplex = true;
		para.bReturnPhysicalMaterial = true;
		if (GetWorld()->LineTraceSingleByChannel(Hit, eyeLoc, traceEnd, TRACECHANNEL_WEAPON, para)) {
			// 处理命中效果
			dealHit(Hit);
		}
		PlayFireEffect();
	}
}

void AFPSHeroWeapon::ApplyNewRecoilCameraOffset(float Pitch, float Yaw)
{
	if (!Owner)
		return;
	APlayerController* controller = Cast<APlayerController>(Owner->GetController());
	if (controller) {
		// 取反使得正值为镜头向上
		controller->AddPitchInput(-(Pitch - PitchOffset));
		// 取反使得正值为镜头向右
		controller->AddYawInput(-(Yaw - YawOffset));
		PitchOffset = Pitch;
		YawOffset = Yaw;
	}
}
