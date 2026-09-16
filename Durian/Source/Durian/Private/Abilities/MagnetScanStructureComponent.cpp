#include "Abilities/MagnetScanStructureComponent.h"

#include "Components/PrimitiveComponent.h"

void UMagnetScanStructureComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Owner->GetComponents(PrimitiveComponents);

		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (IsValid(PrimitiveComponent))
			{
				PrimitiveComponent->SetRenderCustomDepth(true);
				PrimitiveComponent->SetCustomDepthStencilValue(StructureStencilValue);
			}
		}
	}
}
