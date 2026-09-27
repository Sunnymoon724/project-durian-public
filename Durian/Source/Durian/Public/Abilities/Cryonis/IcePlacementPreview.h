#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IcePlacementPreview.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS()
class DURIAN_API AIcePlacementPreview : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AIcePlacementPreview();

	virtual void Tick(float DeltaSeconds) override;

	void SetPreviewState(const FVector& Location, bool bVisible, bool bCanSpawn);
	void ClearPreview();

private:
	bool IsSetupValid() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewPlane;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewPillar;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	float PillarPreviewElapsed = 0.0f;
	FVector LastPreviewLocation = FVector::ZeroVector;
	bool bHasPreviewLocation = false;
	
	float PillarHeight = 1.0f;
	float PillarHorizontalScale = 1.0f;
	float PillarAnimationDuration = 1.0f;

};
