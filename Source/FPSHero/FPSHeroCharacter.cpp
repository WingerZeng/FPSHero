// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSHeroCharacter.h"
#include <algorithm>
#include "FPSHeroGrenade.h"
#include "FPSHeroWeapon.h"
#include "FPSHeroGrenade.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void AFPSHeroCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	auto controller = Cast<APlayerController>(GetController());
	OutLocation = controller->PlayerCameraManager->GetCameraLocation();
	OutRotation = controller->PlayerCameraManager->GetCameraRotation();
}

EWeaponSlot AFPSHeroCharacter::GetWeaponSlot(AFPSHeroWeaponBase* Weapon)
{
	if (Cast<AFPSHeroWeapon>(Weapon)) {
		return EWeaponSlot::RIFLE_SLOT;
	}
	else if(Cast<AFPSHeroGrenade>(Weapon)){
		return EWeaponSlot::GRENADE_SLOT;
	}
	else {
		return EWeaponSlot::RIFLE_SLOT;
	}
}

EWeaponSlot AFPSHeroCharacter::GetNextWeaponSlot(EWeaponSlot WeaponSlot)
{
	if (WeaponSlot == EWeaponSlot::RIFLE_SLOT)
		return EWeaponSlot::GRENADE_SLOT;
	if (WeaponSlot == EWeaponSlot::GRENADE_SLOT)
		return EWeaponSlot::RIFLE_SLOT;
	return EWeaponSlot::GRENADE_SLOT;
}

EWeaponSlot AFPSHeroCharacter::FindNextValidSlot(EWeaponSlot WeaponSlot)
{
	EWeaponSlot InitSlot = WeaponSlot;
	do {
		if (Weapons[WeaponSlot] && Weapons[WeaponSlot]->IsValidLowLevel())
			return WeaponSlot;
		WeaponSlot = GetNextWeaponSlot(WeaponSlot);
	} while (WeaponSlot != InitSlot);
	return WeaponSlot;
}

bool AFPSHeroCharacter::FindWeapon(AFPSHeroWeaponBase* Weapon, EWeaponSlot& WeaponSlot)
{
	for (const auto& WeaponIt : Weapons) {
		if (WeaponIt.Value == Weapon)
		{
			WeaponSlot = WeaponIt.Key;
			return true;
		}
	}
	return false;
}

EWeaponSlot AFPSHeroCharacter::GetWeaponTypeSlot(TSubclassOf<AFPSHeroWeaponBase> WeaponType)
{
	if (TSubclassOf<AFPSHeroWeapon>(WeaponType)) {
		return EWeaponSlot::RIFLE_SLOT;
	}
	else if (TSubclassOf<AFPSHeroGrenade>(WeaponType)) {
		return EWeaponSlot::GRENADE_SLOT;
	}
	else {
		return EWeaponSlot::RIFLE_SLOT;
	}
}

AFPSHeroCharacter::AFPSHeroCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraHolder = CreateDefaultSubobject<USceneComponent>(TEXT("FirstPersonCameraHolder"));
	FirstPersonCameraHolder->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraHolder->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetupAttachment(FirstPersonCameraHolder);

	ThirdPersonSpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
	ThirdPersonSpringArmComp->SetupAttachment(GetCapsuleComponent());

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	FPSWeaponSocketName = "FPSGripPoint";
	TPSWeaponSocketName = "TPSGripPoint";
	DefaultViewMode = EViewMode::FIRST_PERSON;
	DefaultWeaponSlot = EWeaponSlot::RIFLE_SLOT;
	Weapons.Add(EWeaponSlot::GRENADE_SLOT, nullptr);
	Weapons.Add(EWeaponSlot::RIFLE_SLOT, nullptr);
	bShouldInitAnim = false;

	this->PrimaryActorTick.bCanEverTick = true;
}

AFPSHeroWeaponBase* AFPSHeroCharacter::GetWeapon()
{
	if (!Weapons.Find(CurrentWeaponSlot) || !Weapons[CurrentWeaponSlot]->IsValidLowLevel())
		return nullptr;
	return Weapons[CurrentWeaponSlot];
}

void AFPSHeroCharacter::GripWeapon(TSubclassOf<class AFPSHeroWeaponBase> GrappedWeaponType)
{
	EWeaponSlot ThisSlot = GetWeaponTypeSlot(GrappedWeaponType);
	ThrowWeapon(ThisSlot);
	FActorSpawnParameters paras;
	paras.Owner = this;
	paras.Instigator = this;
	paras.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Weapons[ThisSlot] = GetWorld()->SpawnActor<AFPSHeroWeaponBase>(GrappedWeaponType, FTransform(), paras);
	Weapons[ThisSlot]->SetOwnerCharacter(this, ThisSlot);

	if (ThisSlot == CurrentWeaponSlot)
	{
		SwitchToWeaponSlot(ThisSlot);
	}
	else {
		Weapons[ThisSlot]->SetWeaponActive(false);
	}
}

void AFPSHeroCharacter::ThrowWeapon(EWeaponSlot WeaponSlot)
{
	if (Weapons[WeaponSlot]) {
		Weapons[WeaponSlot]->Throw();
		Weapons[WeaponSlot] = nullptr;
	}
}

bool AFPSHeroCharacter::SwitchToWeaponSlot(EWeaponSlot WeaponSlot)
{
	if (bIsFiring)
		EndFire(EFireEndReason::SWAP_WEAPON);
	bIsFiring = false;
	if (!Weapons[WeaponSlot]) {
		return false;
	}
	if (Weapons[CurrentWeaponSlot]) {
		Weapons[CurrentWeaponSlot]->SetWeaponActive(false);
		Weapons[CurrentWeaponSlot]->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	FAttachmentTransformRules rule(EAttachmentRule::SnapToTarget, true);
	CurrentWeaponSlot = WeaponSlot;
	Weapons[CurrentWeaponSlot]->AttachToComponent(GetCurrentMesh(), rule, (ViewMode == EViewMode::FIRST_PERSON ? FPSWeaponSocketName : TPSWeaponSocketName));
	Weapons[CurrentWeaponSlot]->SetWeaponActive(true);
	UpdateAnimationClass();
	return true;
}

void AFPSHeroCharacter::SetViewMode(EViewMode NewViewMode)
{
	ViewMode = NewViewMode;
	if (ViewMode == EViewMode::FIRST_PERSON) {
		GetMesh()->SetVisibility(false);
		Mesh1P->SetVisibility(true);
		ThirdPersonSpringArmComp->bUsePawnControlRotation = false;
		ThirdPersonSpringArmComp->SetRelativeRotation(FRotator());
		FirstPersonCameraComponent->ResetRelativeTransform();
		FirstPersonCameraComponent->AttachToComponent(FirstPersonCameraHolder, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
	}
	else if (ViewMode == EViewMode::THIRD_PERSON) {
		Mesh1P->SetVisibility(false);
		GetMesh()->SetVisibility(true);
		FirstPersonCameraComponent->bUsePawnControlRotation = false;
		FirstPersonCameraComponent->ResetRelativeTransform();
		ThirdPersonSpringArmComp->SetRelativeRotation(FRotator());
		FirstPersonCameraComponent->AttachToComponent(ThirdPersonSpringArmComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ThirdPersonSpringArmComp->bUsePawnControlRotation = true;
	}
	SwitchToWeaponSlot(CurrentWeaponSlot);
	bIsFiring = false;
}

EViewMode AFPSHeroCharacter::GetViewMode()
{
	return ViewMode;
}

void AFPSHeroCharacter::SwitchViewMode()
{
	if (ViewMode == EViewMode::FIRST_PERSON) {
		SetViewMode(EViewMode::THIRD_PERSON);
	}
	else if (ViewMode == EViewMode::THIRD_PERSON) {
		SetViewMode(EViewMode::FIRST_PERSON);
	}
}

void AFPSHeroCharacter::UpdateAnimationClass()
{
	bShouldInitAnim = true;
	if (GetCurrentMesh()->IsPostEvaluatingAnimation())
		return;
	AFPSHeroWeaponBase* CurWeapon = GetWeapon();
	TSubclassOf<UAnimInstance> NewAnim;
	if (CurWeapon) {
		NewAnim = CurWeapon->GetAnimClass(ViewMode);
	}
	if (!NewAnim) {
		NewAnim = GetDefaultAnimClass(ViewMode);
	}
	if (NewAnim) {
		GetCurrentMesh()->SetAnimClass(NewAnim);
	}
	bShouldInitAnim = false;
}

UAnimInstance* AFPSHeroCharacter::GetAnimInstance()
{
	if (ViewMode == EViewMode::FIRST_PERSON) {
		return Mesh1P->GetAnimInstance();
	}
	else if (ViewMode == EViewMode::THIRD_PERSON) {
		return GetMesh()->GetAnimInstance();
	}
	return Mesh1P->GetAnimInstance();
}

void AFPSHeroCharacter::PlayMontage(UAnimMontage* Montage)
{
	if (Montage != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(Montage);
		}
	}
}

USkeletalMeshComponent* AFPSHeroCharacter::GetCurrentMesh()
{
	if (ViewMode == EViewMode::FIRST_PERSON) {
		return Mesh1P;
	}
	else if (ViewMode == EViewMode::THIRD_PERSON) {
		return GetMesh();
	}
	return Mesh1P;
}

bool AFPSHeroCharacter::IsFiring()
{
	return bIsFiring;
}

FRotator AFPSHeroCharacter::GetAimingRotation()
{
	if (ViewMode == EViewMode::FIRST_PERSON)
		return GetControlRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
	DeltaRot.Yaw = UKismetMathLibrary::FClamp(DeltaRot.Yaw, -90.0f, 90.0f);
	return DeltaRot + GetActorRotation();
}

TSubclassOf<UAnimInstance> AFPSHeroCharacter::GetDefaultAnimClass(EViewMode AnimViewMode)
{
	if (AnimViewMode == EViewMode::FIRST_PERSON) {
		return DefaultFPSAnimClass;
	}
	else
		return DefaultTPSAnimClass;
}

void AFPSHeroCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	CurrentWeaponSlot = DefaultWeaponSlot;

	for (const auto& WeaponType : DefaultWeaponTypes) {
		GripWeapon(WeaponType);
	}
	SwitchToWeaponSlot(FindNextValidSlot(DefaultWeaponSlot));
	SetViewMode(DefaultViewMode);
}

void AFPSHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bShouldInitAnim)
		UpdateAnimationClass();
	if (bIsFiring && IsViewBack()) {
		EndFire(EFireEndReason::FACE_TO_BACK);
		if (FireInterruptedSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireInterruptedSound, GetActorLocation());
	}
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
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSHeroCharacter::EndFireByRelease);
	PlayerInputComponent->BindAction("SwitchFireMode", IE_Pressed, this, &AFPSHeroCharacter::SwitchFireMode);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSHeroCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSHeroCharacter::MoveRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSHeroCharacter::Jump);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSHeroCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSHeroCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("SwitchViewMode", IE_Pressed, this, &AFPSHeroCharacter::SwitchViewMode);
	PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &AFPSHeroCharacter::SwitchToWeaponSlot1);
	PlayerInputComponent->BindAction("Weapon2", IE_Pressed, this, &AFPSHeroCharacter::SwitchToWeaponSlot2);
}

void AFPSHeroCharacter::OnFire()
{
	if (IsViewBack()) {
		SetActorRotation(FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, GetActorRotation().Roll));
		bIsForceToFireOrientaion = true;
	}
	bIsFiring = true;
	if (Weapons[CurrentWeaponSlot]) {
		Weapons[CurrentWeaponSlot]->Fire();
	}
}

void AFPSHeroCharacter::EndFire(EFireEndReason Reason)
{
	bIsFiring = false;
	if (Weapons[CurrentWeaponSlot]) {
		Weapons[CurrentWeaponSlot]->EndFire(Reason);
	}
}

void AFPSHeroCharacter::EndFireByRelease()
{
	EndFire(EFireEndReason::MOUSE_REALEASE);
}

void AFPSHeroCharacter::SwitchFireMode()
{
	if (Weapons[CurrentWeaponSlot]) {
		Weapons[CurrentWeaponSlot]->SwitchFireMode();
		EndFire(EFireEndReason::SWITCH_FIRE_MODE);
	}
}

void AFPSHeroCharacter::SwitchToWeaponSlot1()
{
	SwitchToWeaponSlot(EWeaponSlot::RIFLE_SLOT);
}

void AFPSHeroCharacter::SwitchToWeaponSlot2()
{
	SwitchToWeaponSlot(EWeaponSlot::GRENADE_SLOT);
}

void AFPSHeroCharacter::RemoveWeapon(EWeaponSlot WeaponSlot)
{
	Weapons[WeaponSlot] = nullptr;
	if (CurrentWeaponSlot == WeaponSlot) {
		WeaponSlot = FindNextValidSlot(WeaponSlot);
		SwitchToWeaponSlot(WeaponSlot);
	}
}

void AFPSHeroCharacter::MoveForward(float Value)
{
	GetCharacterMovement()->bOrientRotationToMovement = Value > 0;
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, FirstPersonCameraComponent->GetComponentRotation().Yaw, 0)), Value);
	}
}

void AFPSHeroCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, FirstPersonCameraComponent->GetComponentRotation().Yaw, 0)), Value);
	}
}

bool AFPSHeroCharacter::IsViewBack()
{
	if (ViewMode == EViewMode::FIRST_PERSON)
		return false;
	FRotator YawRotator(0, GetControlRotation().Yaw, 0);
	FRotator ActorYawRotator(0, GetActorRotation().Yaw, 0);
	return FVector::DotProduct(YawRotator.Vector(), ActorYawRotator.Vector()) < 0;
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