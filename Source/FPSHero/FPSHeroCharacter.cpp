// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSHeroCharacter.h"
#include "FPSHeroProjectile.h"
#include "FPSHeroWeapon.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void AFPSHeroCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	auto controller = Cast<APlayerController>(GetController());
	OutLocation = controller->PlayerCameraManager->GetCameraLocation();
	OutRotation = controller->PlayerCameraManager->GetCameraRotation();
}

//////////////////////////////////////////////////////////////////////////
// AFPSHeroCharacter

AFPSHeroCharacter::AFPSHeroCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	WeaponSocketName = "GripPoint";

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

AFPSHeroWeapon* AFPSHeroCharacter::GetWeapon()
{
	return Weapon;
}

void AFPSHeroCharacter::GrapWeapon(TSubclassOf<class AFPSHeroWeapon> GrappedWeaponType)
{
	if (Weapon) {
		Weapon->Destroy();
	}

	FActorSpawnParameters paras;
	paras.Owner = this;
	paras.Instigator = this;
	paras.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Weapon = GetWorld()->SpawnActor<AFPSHeroWeapon>(GrappedWeaponType, FTransform(), paras);
	Weapon->SetOwner(this);
	FAttachmentTransformRules rule(EAttachmentRule::SnapToTarget, true);
	Weapon->AttachToComponent(Mesh1P, rule, WeaponSocketName);
}

void AFPSHeroCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (WeaponType) {
		GrapWeapon(WeaponType);
	}

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFPSHeroCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSHeroCharacter::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSHeroCharacter::EndFire);
	PlayerInputComponent->BindAction("SwitchFireMode", IE_Pressed, this, &AFPSHeroCharacter::SwitchFireMode);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSHeroCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSHeroCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSHeroCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSHeroCharacter::LookUpAtRate);
}

void AFPSHeroCharacter::OnFire()
{
	// try and fire a projectile
	if (Weapon) {
		Weapon->Fire();
	}
}

void AFPSHeroCharacter::EndFire()
{
	// try and fire a projectile
	if (Weapon) {
		Weapon->EndFire();
	}
}

void AFPSHeroCharacter::SwitchFireMode()
{
	if (Weapon) {
		Weapon->SwitchFireMode();
	}
}

void AFPSHeroCharacter::DoFire()
{
	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AFPSHeroCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSHeroCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSHeroCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSHeroCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}