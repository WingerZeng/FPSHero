// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FPSHero.h"
#include "GameFramework/Character.h"
#include "FPSHeroWeaponBase.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "FPSHeroCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AFPSHeroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSHeroCharacter();
	
	UFUNCTION(BlueprintCallable)
	AFPSHeroWeaponBase* GetCurrentWeapon();

	UFUNCTION(BlueprintCallable)
		AFPSHeroWeaponBase* GetWeapon(EWeaponSlot Slot);

	UFUNCTION(BlueprintCallable)
		void SetWeapon(EWeaponSlot Slot, AFPSHeroWeaponBase* Weapon);

	UFUNCTION(Server, BlueprintCallable, Reliable)
		void GripWeapon(TSubclassOf<AFPSHeroWeaponBase> GrappedWeaponType);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void ThrowWeapon(EWeaponSlot WeaponSlot, bool bShouldChangeWeaponAuto = false);

	UFUNCTION()
		void OnWeaponUpdate();

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
		void SwitchToWeaponSlot(EWeaponSlot WeaponSlot);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
		void RemoveWeapon(EWeaponSlot WeaponSlot);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void SetViewModeServer(EViewMode NewViewMode);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void SetViewModeNetMulticast(EViewMode NewViewMode);

	void SetViewMode(EViewMode NewViewMode);

	EViewMode GetViewMode();

	EViewMode GetDisplayViewMode();

	UFUNCTION(BlueprintCallable)
		void SwitchViewMode();

	UFUNCTION(BlueprintCallable)
		void UpdateAnimationClass();

	UFUNCTION(BlueprintCallable)
		UAnimInstance* GetAnimInstance();

	UFUNCTION(BlueprintCallable)
		void PlayMontage(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable)
		USkeletalMeshComponent* GetCurrentMesh();

	UFUNCTION(BlueprintCallable)
		bool IsTurning();

	UFUNCTION(BlueprintCallable)
	EWeaponSlot GetWeaponTypeSlot(TSubclassOf<AFPSHeroWeaponBase> WeaponType);

	UFUNCTION(BlueprintCallable)
		bool IsFiring();

	UFUNCTION(BlueprintCallable)
		FRotator GetAimingRotation();

	UFUNCTION(BlueprintCallable)
		TSubclassOf<UAnimInstance> GetDefaultAnimClass(EViewMode ViewMode);

	UFUNCTION(BlueprintCallable)
		float GetHealth();

	UFUNCTION(BlueprintCallable)
		void SetHealth(float Health);

	UFUNCTION(BlueprintCallable)
		float GetAttack();

	UFUNCTION(BlueprintCallable)
		void SetAttack(float NewAttack);

	UFUNCTION(BlueprintCallable)
		float GetDefence();

	UFUNCTION(BlueprintCallable)
		void SetDefence(float NewDefence);

	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual FRotator GetViewRotation() const override;

	UFUNCTION(BlueprintCallable)
	FRotator GetCharacterViewRotation() const;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<class AFPSHeroWeaponBase>> DefaultWeaponTypes;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponSlot DefaultWeaponSlot;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		EViewMode DefaultViewMode;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		TSubclassOf<UAnimInstance> DefaultFPSAnimClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		TSubclassOf<UAnimInstance> DefaultTPSAnimClass;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	void EndFire(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE);

	void OnFireButtonPress();

	void OnFireButtonRelease();

	UFUNCTION(Server, Reliable)
	void OnFireServer();

	UFUNCTION(Server, Reliable)
	void EndFireServer(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE);

	UFUNCTION(NetMulticast, Reliable)
	void OnFireMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void EndFireMulticast(EFireEndReason Reason = EFireEndReason::MOUSE_REALEASE);

	UFUNCTION(Server, Reliable)
	void SwitchFireMode();

	UFUNCTION(Server, Reliable)
	void SwitchToWeaponSlot1();

	UFUNCTION(Server, Reliable)
	void SwitchToWeaponSlot2();

	// Set View Rotation For Other Client
	UFUNCTION(NetMulticast, Unreliable)
	void SetCharacterViewRotation(FRotator Rotator);
	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);
	// UFUNCTION(Server, Reliable)
	// void TurnAtRateServer(float Rate);
	// UFUNCTION(NetMulticast, Reliable)
	// void TurnAtRateMulticast(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	// UFUNCTION(Server, Reliable)
	// void LookUpAtRateServer(float Rate);
	// UFUNCTION(NetMulticast, Reliable)
	// void LookUpAtRateMulticast(float Rate);
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsForceToFireOrientaion = false;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_Weapons, VisibleAnywhere, BlueprintReadOnly)
		TArray<AFPSHeroWeaponBase*> Weapons;
	UFUNCTION()
		void OnRep_Weapons();

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
		FName FPSWeaponSocketName;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
		FName TPSWeaponSocketName;

	EWeaponSlot GetWeaponSlot(AFPSHeroWeaponBase* Weapon);

	EWeaponSlot GetNextWeaponSlot(EWeaponSlot WeaponSlot);

	EWeaponSlot FindValidSlotFromSlot(EWeaponSlot WeaponSlot);
	
	EWeaponSlot FindValidSlotFromNextSlot(EWeaponSlot WeaponSlot);

	virtual void OnHealthUpdate();

	virtual void OnDie();

	bool FindWeapon(AFPSHeroWeaponBase* const Weapon, EWeaponSlot& WeaponSlot);

	virtual void Initialize();
		 
		/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USceneComponent* FirstPersonCameraHolder;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* ThirdPersonSpringArmComp;

	EViewMode ViewMode;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponSlot)
	EWeaponSlot CurrentWeaponSlot;
	UFUNCTION()
	void OnRep_CurrentWeaponSlot();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	bool bIsFiring;

	bool bShouldInitAnim;

	float TurnThresholdStartAngle = 80;

	float TurnThresholdStopAngle = 30;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float TurnRate = 10;

	UPROPERTY(Replicated)
	bool bIsTurning = false;

	// Attributes
	UPROPERTY(EditDefaultsOnly, Category = BaseAttributes)
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	UFUNCTION()
		void OnRep_CurrentHealth();

	UPROPERTY(EditDefaultsOnly, Category = BaseAttributes)
		float DefaultAttack;

	UPROPERTY(Replicated)
		float Attack;

	UPROPERTY(EditDefaultsOnly, Category = BaseAttributes)
		float DefaultDefence;

	UPROPERTY(Replicated)
		float Defence;

	UPROPERTY(ReplicatedUsing = OnRep_Initialzed)
		bool bInitialized;
	UFUNCTION()
	void OnRep_Initialzed();

	FRotator CharacterViewRotation;
};


