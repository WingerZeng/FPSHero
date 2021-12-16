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

	virtual void OnAmmoUpdate();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	int GetAmmo() const;
	
	UFUNCTION(BlueprintCallable)
	void SetAmmo(int Ammo);
	
	UFUNCTION(BlueprintCallable)
	int GetTotalAmmo() const;
	
	UFUNCTION(BlueprintCallable)
	void SetTotalAmmo(int TotalAmmo);

	UFUNCTION(BlueprintCallable)
	EWeaponSlot GetSlot();
	
protected:
	UFUNCTION()
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

	UPROPERTY(ReplicatedUsing = OnRep_IsWeaponActive, VisibleAnywhere, BlueprintReadOnly)
		bool bIsWeaponActive = false;
	UFUNCTION()
	void OnRep_IsWeaponActive();

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Weapon")
		FireMode Mode;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		bool IsFireModeLocked;
	
private:
	UPROPERTY(Replicated, VisibleAnywhere)
		AFPSHeroCharacter* OwnerCharacter = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_Ammo, BlueprintReadOnly)
	int Ammo;
	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_TotalAmmo, BlueprintReadOnly)
	int TotalAmmo;
	UFUNCTION()
	void OnRep_TotalAmmo();

	UPROPERTY(EditDefaultsOnly)
	EWeaponSlot EquipSlot;
};