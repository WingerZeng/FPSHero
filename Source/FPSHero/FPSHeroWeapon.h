#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSHeroWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;
class UDefaultCameraShakeBase;
class UDamageType;
class UFPSHeroCameraShakePattern;
class AFPSHeroCharacter;
class UCameraShakePattern;
class USceneComponent;
class USoundBase;
struct FHitResult;

UENUM()
enum class FireMode
{
	None = 0,
	Single = 1,
	Auto = 2
};


UCLASS(BlueprintType)
class FPSHERO_API AFPSHeroWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFPSHeroWeapon();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category="Component")
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Component")
		USceneComponent* Root;

public:
	void SetOwner(AFPSHeroCharacter* MyOwner);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EndFire();

	void SwitchFireMode();

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void dealHit(const FHitResult& Hit);

	void PlayFireEffect();

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
		FName MussleName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float ShootIntervalSecond;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		FireMode Mode;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool IsFireModeLocked;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
		float RestoreSecond;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
		TSubclassOf<UCameraShakeBase> CameraShakeType;

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

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		float BodyDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		float HeadDamage;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		void GetRecoilPitchWithTime(float time, float& pitch);

	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		//horizon or vertical equal to 1 means 45¡ãspread
		void GetRecoilDirectionalSpreadWithTime(float time, float& SpreadUp, float& SpreadRight);

	UFUNCTION(BlueprintImplementableEvent, Category = "Recoil")
		//horizon or vertical equal to 1 means 45¡ãspread
		void GetRecoilRandomSpreadWithTime(float time, float& scale);

	virtual void SingleFire();

	void ApplyRecoilPitch(float pitch);

private:
	AFPSHeroCharacter* Owner = nullptr;

	//For Automatic
	float FireTimer;
	bool IsFiring;
	float SecondsSinceStartFire;
	float SecondsSinceStopFire;
	float FireTimeForRecoil;
	float FireTimeWhenStop;
};

