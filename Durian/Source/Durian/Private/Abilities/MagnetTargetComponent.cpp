// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/MagnetTargetComponent.h"

#include "Components/PrimitiveComponent.h"

// Sets default values for this component's properties
UMagnetTargetComponent::UMagnetTargetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
}


// Called when the game starts
void UMagnetTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetOwner()->GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent)
		{
			PrimitiveComponent->SetRenderCustomDepth(true);
			PrimitiveComponent->SetCustomDepthStencilValue(1);
		}
	}
}
