// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "ButtonBoxComponent.generated.h"
class UButtonBoxComponent;

//×¢²ábutton´¥·¢ÊÂ¼þ
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FButtonHitSignature, UButtonBoxComponent, OnButtonHit, UPrimitiveComponent*, HitComponent);

UCLASS(ClassGroup = "Collision", hidecategories = (Object, LOD, Lighting, TextureStreaming), editinlinenew, meta = (DisplayName = "Button Box Collision", BlueprintSpawnableComponent))
class FPSHERO_API UButtonBoxComponent : public UBoxComponent
{
	GENERATED_BODY()
public:
	UButtonBoxComponent();

	UFUNCTION(BlueprintCallable)
	void HitButton();

	UPROPERTY(BlueprintAssignable, Category = "Collision")
	FButtonHitSignature OnButtonHit;
};
