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
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"

AFPSHeroWeapon::AFPSHeroWeapon()
{
	MussleName = "MuzzleSocket";

	BodyDamage = 50;
	HeadDamage = 100;

	ShootIntervalSecond = 1;
	
	Mode = FireMode::Single;
}

void AFPSHeroWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(!IsFiring){
		if (GetOwnerCharacter() && GetOwnerCharacter()->GetLocalRole() == ROLE_Authority) {
			// 停止射击后，计算后坐力恢复和准心恢复
			SecondsSinceStopFire += DeltaSeconds;
			// 后坐力恢复
			if (CurrentFiredAmmo > 0 && RecoilInstance) {
				CurrentFiredAmmo = FGenericPlatformMath::Max(0.0f,
					FGenericPlatformMath::CeilToFloat((RecoilInstance->RecoilRestoreTime - SecondsSinceStopFire)
						/ RecoilInstance->RecoilRestoreTime * FiredAmmoWhenStop));
			}
		}
		// 准心恢复
		if (GetOwnerCharacter() && GetOwnerCharacter()->IsLocallyControlled())
		{
			float RestoreRatio = 1;
			if (RecoilInstance)
				RecoilInstance->GetCameraRestoreRatio(SecondsSinceStopFire, RestoreRatio);
			float TargetPicth = PitchOffsetWhenStop * (1 - RestoreRatio);
			float TargetYaw = YawOffsetWhenStop * (1 - RestoreRatio);
			ApplyNewRecoilCameraOffset(TargetPicth, TargetYaw);
		}
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

void AFPSHeroWeapon::OnRep_LastHitResult()
{
	PlayHitEffect(LastHitResult);
}

void AFPSHeroWeapon::DealHit_Implementation(const FHitResult& Hit)
{
	EPhysicalSurface physType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	FVector eyeLoc;
	FRotator eyeRot;
	GetOwnerCharacter()->GetActorEyesViewPoint(eyeLoc, eyeRot);

	switch (physType) {
	case PHYSMAT_FLESH: //default flesh
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), BodyDamage, Hit.ImpactPoint - eyeLoc, Hit, GetOwnerCharacter()->GetController(), this, DamageType);
		break;
	case PHYSMAT_FLESHVULNERABLE: //vulnerable flesh
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), HeadDamage, Hit.ImpactPoint - eyeLoc, Hit, GetOwnerCharacter()->GetController(), this, DamageType);
		break;
	default:
		UButtonBoxComponent* box = Cast<UButtonBoxComponent>(Hit.Component);
		if (box)
			box->HitButton(GetOwnerCharacter()->GetController());
		break;
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
	APlayerController* Controller = Cast<APlayerController>(GetOwnerCharacter()->GetController());

	if (GetOwnerCharacter() && GetOwnerCharacter()->IsLocallyControlled() && RecoilInstance) {
		// 镜头抖动
		RecoilInstance->ApplyCameraShake(CurrentFiredAmmo, Controller);
		// 镜头移动
		float RecoilPitch, RecoilYaw;
		RecoilInstance->GetCameraMovement(CurrentFiredAmmo, RecoilPitch, RecoilYaw);
		ApplyNewRecoilCameraOffset(RecoilPitch, RecoilYaw);
	}

	if (GetOwnerCharacter()) {
		GetOwnerCharacter()->PlayMontage(FireMontage);
	}
}

void AFPSHeroWeapon::PlayHitEffect(const FHitResult& Hit)
{
	EPhysicalSurface physType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	UParticleSystem* EffectToPlay;

	switch (physType) {
	case PHYSMAT_FLESH: //default flesh
		EffectToPlay = DefaultFleshEffect;
		break;
	case PHYSMAT_FLESHVULNERABLE: //vulnerable flesh
		EffectToPlay = HeadshotEffect;
		break;
	default:
		EffectToPlay = DefaultImpactEffect;
		break;
	}

	if (EffectToPlay) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EffectToPlay, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
}

void AFPSHeroWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSHeroWeapon, LastHitResult);
}

void AFPSHeroWeapon::SingleFire()
{
	if(GetAmmo() == 0)
		return;
	SetTotalAmmo(GetTotalAmmo()-1);
	SetAmmo(GetAmmo()-1);
	
	CurrentFiredAmmo++;
	
	if (GetOwnerCharacter()) {
		PlayFireEffect();

		if (GetOwnerCharacter()->GetLocalRole() == ROLE_Authority) {
			// 得到摄像机的位置和朝向
			FVector eyeLoc;
			FRotator eyeRot;
			GetOwnerCharacter()->GetActorEyesViewPoint(eyeLoc, eyeRot);
			FVector eyeDir = eyeRot.Vector();

			/* 后坐力扩散 */
			// 方向性扩散
			// 首先得到镜头的Up、Right方向在世界坐标系下的方向向量，再乘上对应方向的扩散系数
			// #TEST
			//eyeDir.Normalize();
			//FVector UpDir(0, 0, 1);
			//FVector RightDir = FVector::CrossProduct(eyeDir, UpDir);
			//RightDir.Normalize();
			//float SpreadUp = 0, SpreadRight = 0;
			//if (RecoilInstance) {
			//	RecoilInstance->GetDirectionalSpread(CurrentFiredAmmo, SpreadUp, SpreadRight);
			//}
			//eyeDir += UpDir * SpreadUp + RightDir * SpreadRight;
			//eyeDir.Normalize();
			//// 随机扩散
			//// 根据扩散系数，在圆锥上取随机向量
			//float SpreadScale = 0;
			//if (RecoilInstance) {
			//	RecoilInstance->GetRandomSpread(CurrentFiredAmmo, SpreadScale);
			//}
			//eyeDir = UKismetMathLibrary::RandomUnitVectorInConeInRadians(eyeDir, SpreadScale);

			// 得到光线追踪目标点
			FVector traceEnd = eyeLoc + eyeDir * 100000;
			FHitResult TempHit;
			FCollisionQueryParams para;
			// 忽略角色和枪模型
			para.AddIgnoredActor(GetOwnerCharacter());
			para.AddIgnoredActor(this);
			// 使用复杂碰撞来求交
			para.bTraceComplex = true;
			para.bReturnPhysicalMaterial = true;
			if (GetWorld()->LineTraceSingleByChannel(TempHit, eyeLoc, traceEnd, TRACECHANNEL_WEAPON, para)) {
				LastHitResult = TempHit;
				// 处理命中
				DealHit(LastHitResult);
				// 命中效果
				OnRep_LastHitResult();
			}
		}
	}
}

void AFPSHeroWeapon::ApplyNewRecoilCameraOffset(float Pitch, float Yaw)
{
	if (!GetOwnerCharacter())
		return;
	APlayerController* controller = Cast<APlayerController>(GetOwnerCharacter()->GetController());
	if (controller) {
		// 取反使得正值为镜头向上
		controller->AddPitchInput(-(Pitch - PitchOffset));
		// 取反使得正值为镜头向右
		controller->AddYawInput(-(Yaw - YawOffset));
		PitchOffset = Pitch;
		YawOffset = Yaw;
	}
}
