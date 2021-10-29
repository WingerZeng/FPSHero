#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSHeroWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;
class UCameraShake;
class UDamageType;
struct FHitResult;

UCLASS()
class FPSHERO_API AFPSHeroWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFPSHeroWeapon();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	USkeletalMeshComponent* MeshComp;

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void dealHit(const FHitResult& Hit);

	void PlayFireEffect();

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName MussleName;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* MussleEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* DefaultFleshEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* HeadshotEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		TSubclassOf<UCameraShake> ShakeType;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		float BodyDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		float HeadDamage;
};

