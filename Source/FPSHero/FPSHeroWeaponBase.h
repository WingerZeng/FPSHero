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
	
	UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent)
	void SetWeaponActive(bool bActive);

	void Throw();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	FVector GetLeftHandSocketPosition();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void Fire(){};

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void EndFire(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE) {};

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		bool ShouldUseLeftHand() { return ShouldHoldByTwoHands; }

	UFUNCTION(BlueprintCallable)
		AFPSHeroCharacter* GetOwnerCharacter() { return OwnerCharacter; };

	UFUNCTION(BlueprintCallable)
		TSubclassOf<UAnimInstance> GetAnimClass(EViewMode ViewMode);

	UFUNCTION(NetMulticast, Reliable)
	void DetachFromCharacterMulticast();
	
	virtual void AttachToCharacter(class USkeletalMeshComponent* Mesh, FName SocketName);

	virtual void DetachFromCharacter();

protected:
	virtual void SetWeaponActive_Implementation(bool bActive);

	UFUNCTION()
	virtual void OnActiveStateChanged();

	UPROPERTY(EditDefaultsOnly, Category = "Component")
		USkeletalMeshComponent* MeshComp;

	//UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	//	USkeletalMeshComponent* MeshCompForAttach;

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

	UPROPERTY(ReplicatedUsing = OnActiveStateChanged, VisibleAnywhere, BlueprintReadOnly)
		bool bIsWeaponActive = false;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Weapon")
		FireMode Mode;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool IsFireModeLocked;

private:
	UPROPERTY(Replicated)
		AFPSHeroCharacter* OwnerCharacter = nullptr;
};