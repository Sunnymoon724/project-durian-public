// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/MagnetTargetComponent.h"

// Sets default values for this component's properties
UMagnetTargetComponent::UMagnetTargetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// ...
}


// Called when the game starts
void UMagnetTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
