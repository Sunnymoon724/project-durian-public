#include "Abilities/MagnetScanFloorComponent.h"

#include "Components/PrimitiveComponent.h"

UMagnetScanFloorComponent::UMagnetScanFloorComponent()
{
	bAutoActivate = true;
}

void UMagnetScanFloorComponent::OnRegister()
{
	Super::OnRegister();
	ApplyFloorStencil();
}

void UMagnetScanFloorComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyFloorStencil();
}

void UMagnetScanFloorComponent::ApplyFloorStencil()
{
	if (AActor* Owner = GetOwner())
	{
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Owner->GetComponents(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (IsValid(PrimitiveComponent))
			{
				PrimitiveComponent->SetRenderCustomDepth(true);
				PrimitiveComponent->SetCustomDepthStencilValue(FloorStencilValue);
			}
		}
	}
}
