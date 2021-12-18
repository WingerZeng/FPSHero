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
			// ֹͣ����󣬼���������ָ���׼�Ļָ�
			SecondsSinceStopFire += DeltaSeconds;
			// �������ָ�
			if (CurrentFiredAmmo > 0 && RecoilInstance) {
				CurrentFiredAmmo = FGenericPlatformMath::Max(0.0f,
					FGenericPlatformMath::CeilToFloat((RecoilInstance->RecoilRestoreTime - SecondsSinceStopFire)
						/ RecoilInstance->RecoilRestoreTime * FiredAmmoWhenStop));
			}
		}
		// ׼�Ļָ�
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
	//ǹ����Ч
	if (MussleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MussleEffect, MeshComp, MussleName);
	}
	//��Ч
	if(FireSound)
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	APlayerController* Controller = Cast<APlayerController>(GetOwnerCharacter()->GetController());

	if (GetOwnerCharacter() && GetOwnerCharacter()->IsLocallyControlled() && RecoilInstance) {
		// ��ͷ����
		RecoilInstance->ApplyCameraShake(CurrentFiredAmmo, Controller);
		// ��ͷ�ƶ�
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
			// �õ��������λ�úͳ���
			FVector eyeLoc;
			FRotator eyeRot;
			GetOwnerCharacter()->GetActorEyesViewPoint(eyeLoc, eyeRot);
			FVector eyeDir = eyeRot.Vector();

			/* ��������ɢ */
			// ��������ɢ
			// ���ȵõ���ͷ��Up��Right��������������ϵ�µķ����������ٳ��϶�Ӧ�������ɢϵ��
			// #TEST
			eyeDir.Normalize();
			FVector UpDir(0, 0, 1);
			FVector RightDir = FVector::CrossProduct(eyeDir, UpDir);
			RightDir.Normalize();
			float SpreadUp = 0, SpreadRight = 0;
			if (RecoilInstance) {
				RecoilInstance->GetDirectionalSpread(CurrentFiredAmmo, SpreadUp, SpreadRight);
			}
			eyeDir += UpDir * SpreadUp + RightDir * SpreadRight;
			eyeDir.Normalize();
			// �����ɢ
			// ������ɢϵ������Բ׶��ȡ�������
			float SpreadScale = 0;
			if (RecoilInstance) {
				RecoilInstance->GetRandomSpread(CurrentFiredAmmo, SpreadScale);
			}
			eyeDir = UKismetMathLibrary::RandomUnitVectorInConeInRadians(eyeDir, SpreadScale);

			// �õ�����׷��Ŀ���
			FVector traceEnd = eyeLoc + eyeDir * 100000;
			FHitResult TempHit;
			FCollisionQueryParams para;
			// ���Խ�ɫ��ǹģ��
			para.AddIgnoredActor(GetOwnerCharacter());
			para.AddIgnoredActor(this);
			// ʹ�ø�����ײ����
			para.bTraceComplex = false;
			para.bReturnPhysicalMaterial = true;
			if (GetWorld()->LineTraceSingleByChannel(TempHit, eyeLoc, traceEnd, TRACECHANNEL_WEAPON, para)) {
				LastHitResult = TempHit;
				// ��������
				DealHit(LastHitResult);
				// ����Ч��
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
		// ȡ��ʹ����ֵΪ��ͷ����
		controller->AddPitchInput(-(Pitch - PitchOffset));
		// ȡ��ʹ����ֵΪ��ͷ����
		controller->AddYawInput(-(Yaw - YawOffset));
		PitchOffset = Pitch;
		YawOffset = Yaw;
	}
}
