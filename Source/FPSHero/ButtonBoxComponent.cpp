// Fill out your copyright notice in the Description page of Project Settings.
#include "ButtonBoxComponent.h"

UButtonBoxComponent::UButtonBoxComponent()
{

}

void UButtonBoxComponent::HitButton(AController* controller)
{
	OnButtonHit.Broadcast(controller);
}
