#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSHero.h"
#include "FPSHeroWeaponBase.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;
class UDamageType;
class AFPSHeroCharacter;
class USceneComponent;
class USoundBase;
class UFPSHeroRecoilBase;
class UAnimInstance;
struct FHitResult;

UENUM()
enum class FireMode
{
	None = 0,
	Single = 1,
	Auto = 2
};

UCLASS(BlueprintType, Blueprintable, Abstract, Category = "Weapon")
class FPSHERO_API AFPSHeroWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AFPSHeroWeaponBase();

	void SetOwnerCharacter(AFPSHeroCharacter* MyOwner, EWeaponSlot WeaponSlot);

	void SwitchFireMode();
	
	UFUNCTION(BlueprintNativeEvent)
	void SetWeaponActive(bool bActive);

	void Throw();

	UFUNCTION(BlueprintCallable)
	FVector GetLeftHandSocketPosition();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void Fire(){};

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void EndFire(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE) {};

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		bool ShouldUseLeftHand() { return ShouldHoldByTwoHands; }

	UFUNCTION(BlueprintCallable)
		AFPSHeroCharacter* GetOwnerCharacter() { return Owner; };

	UFUNCTION(BlueprintCallable)
		TSubclassOf<UAnimInstance> GetAnimClass(EViewMode ViewMode);
public:

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		FireMode Mode;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool IsFireModeLocked;

protected:
	virtual void SetWeaponActive_Implementation(bool bActive);

	UPROPERTY(EditAnywhere, Category = "Component")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	USceneComponent* Root;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName LeftHandSocketName;

	AFPSHeroCharacter* Owner = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool ShouldHoldByTwoHands;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		EWeaponSlot SlotInOwner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSubclassOf<UAnimInstance> AnimClassFPS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSubclassOf<UAnimInstance> AnimClassTPS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWeaponActive = false;
};