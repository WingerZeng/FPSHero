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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

protected:
	virtual void SetWeaponActive_Implementation(bool bActive);

	virtual void OnActiveStateChanged();

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName LeftHandSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool ShouldHoldByTwoHands;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSubclassOf<UAnimInstance> AnimClassFPS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		TSubclassOf<UAnimInstance> AnimClassTPS;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		EWeaponSlot SlotInOwner;

	UPROPERTY(Replicated)
		AFPSHeroCharacter* Owner = nullptr;

	UPROPERTY(ReplicatedUsing = OnActiveStateChanged, VisibleAnywhere, BlueprintReadOnly)
		bool bIsWeaponActive = false;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Weapon")
		FireMode Mode;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool IsFireModeLocked;
};