// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaBotCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Perception/AISense_Damage.h"

AMobaBotCharacter::AMobaBotCharacter()
{
	TurnThresholdStartAngle = 30;
	TurnThresholdStopAngle = 10;
}

void AMobaBotCharacter::SetViewMode(EViewMode NewViewMode)
{
	ViewMode = NewViewMode;
	Mesh1P->SetVisibility(false);
	GetMesh()->SetVisibility(true);
	//对于机器人来说，他们只展示第三人称网格，但是判定瞄准时采用第一人称摄像机
	if (IsLocallyControlled()) {
		ThirdPersonSpringArmComp->bUsePawnControlRotation = false;
		ThirdPersonSpringArmComp->SetRelativeRotation(FRotator());
		FirstPersonCameraComponent->ResetRelativeTransform();
		FirstPersonCameraComponent->AttachToComponent(FirstPersonCameraHolder, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
	}
	else {
		FirstPersonCameraComponent->bUsePawnControlRotation = false;
		FirstPersonCameraComponent->ResetRelativeTransform();
		ThirdPersonSpringArmComp->SetRelativeRotation(FRotator());
		FirstPersonCameraComponent->AttachToComponent(ThirdPersonSpringArmComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ThirdPersonSpringArmComp->bUsePawnControlRotation = true;
	}
	ViewMode = EViewMode::THIRD_PERSON;
	OnWeaponUpdate();
}

void AMobaBotCharacter::OnWeaponUpdate()
{
	for (const auto& Weapon : Weapons) {
		if(Weapon)
			Weapon->DetachFromCharacter();
	}
	if (!GetMesh()) {
		return;
	}
	FName WeaponSocketName;
	if(GetCurrentWeapon())
		GetCurrentWeapon()->AttachToCharacter(GetMesh(), TPSWeaponSocketName);
	UpdateAnimationClass();
}

USkeletalMeshComponent* AMobaBotCharacter::GetCurrentMesh()
{
	return GetMesh();
}
void AMobaBotCharacter::OnFireServer_Implementation()
{
	//Bot的子弹是无限的
	GetCurrentWeapon()->SetTotalAmmo(9999999);
	Super::OnFireServer_Implementation();
}