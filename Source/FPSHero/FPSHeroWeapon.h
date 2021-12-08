#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSHeroWeaponBase.h"
#include "FPSHeroWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;
class UDamageType;
class AFPSHeroCharacter;
class USceneComponent;
class USoundBase;
class UFPSHeroRecoilBase;
struct FHitResult;

UCLASS(BlueprintType)
class FPSHERO_API AFPSHeroWeapon : public AFPSHeroWeaponBase
{
	GENERATED_BODY()

public:
	AFPSHeroWeapon();

	virtual void Tick(float DeltaSeconds) override;

	void PlayFireEffect();

	void PlayHitEffect(const FHitResult& Hit);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void SingleFire();

	void ApplyNewRecoilCameraOffset(float Pitch, float Yaw);

public:
	/* Blueprint Methods */

	virtual void Fire() override;

	virtual void EndFire(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void DealHit(const FHitResult& Hit);

	/* Blueprint Attributes */

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName MussleName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float ShootIntervalSecond;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Recoil")
		UFPSHeroRecoilBase* RecoilInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* MussleEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* DefaultFleshEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* HeadshotEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		float BodyDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		float HeadDamage;

protected:

	//float FireTimer = 0;
	FTimerHandle FireTimer;
	bool IsFiring = false;
	// ???????????????????????????????????????????
	float CurrentFiredAmmo = 0;
	// ???????????????
	float SecondsSinceStopFire = 0;
	// ???????????????????????
	float FiredAmmoWhenStop = 0;
	// ??????????
	float PitchOffset = 0, YawOffset = 0;
	// ?????????????????
	float PitchOffsetWhenStop = 0, YawOffsetWhenStop = 0;

	UPROPERTY(ReplicatedUsing = OnRep_LastHitResult)
		FHitResult LastHitResult;

	UFUNCTION()
	void OnRep_LastHitResult();
};

