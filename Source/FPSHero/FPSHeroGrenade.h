// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSHeroWeaponBase.h"
#include "FPSHeroGrenade.generated.h"

class USoundBase;

UCLASS(config=Game)
class AFPSHeroGrenade : public AFPSHeroWeaponBase
{
	GENERATED_BODY()

protected:
	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = Grenade)
	class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, Category = Movement)
	class UProjectileMovementComponent* ProjectileMovement;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
		float BaseDamage;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
		float InnerRadius;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
		float OuterRadius;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
		float DamageFallOff;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
		TSubclassOf<UDamageType> DamageType;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		float AutoExplodeTime;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		float LauchVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		USoundBase* ExplodeSound;

	/** Projectile movement component */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		UParticleSystem* ExplodeEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation)
		FName ThrowSectionName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation)
		FName ThrowOutNotifyName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation)
		FName ThrowReadyNotifyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Animation)
		UAnimMontage* ThrowMontage;

public:
	AFPSHeroGrenade();

	/** called when projectile hits something */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }

	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	virtual void EndFire(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE) override;

	virtual void Fire() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void DetachFromCharacter() override;
	
protected:
	UFUNCTION(BlueprintCallable, Category = GamePlay)
	void ThrowOut();

	UFUNCTION(NetMulticast, Reliable)
	void SetupThrownMovement();

	virtual void OnActiveStateChanged() override;

	UFUNCTION(BlueprintImplementableEvent, Category = GamePlay)
		void StartPrepare();

	UFUNCTION(NetMulticast, Reliable)
		void StartReleaseMulticast();

	UFUNCTION(BlueprintImplementableEvent, Category = GamePlay)
		void StartRelease();

	UFUNCTION(BlueprintCallable, Category = GamePlay)
		void ReadyToThrow();

	void Explode();

	UFUNCTION(NetMulticast, Reliable)
		void StopFireMontage();

	UFUNCTION(NetMulticast, Reliable)
		void PlayExplodeEffect();

private:
	FTimerHandle ExplodeTimer;

	bool bReadyToThrow;

	UPROPERTY(Replicated)
	bool bThrown = false;
};

