#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IcePlacementPreview.generated.h"

class UMaterialInstanceDynamic;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class DURIAN_API AIcePlacementPreview : public AActor
{
	GENERATED_BODY()

public:
	AIcePlacementPreview();
	virtual void Tick(float DeltaSeconds) override;

	void SetPreviewState(const FVector& Location, bool bVisible, bool bCanSpawn);
	void PlaySpawnEffect();
	void ClearPreview();

private:
	void HideSpawnRing();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewPlane;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewPillar;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SpawnRing;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PillarPreviewMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RingMaterial;

	FTimerHandle SpawnRingTimer;
	float SpawnRingElapsed = 0.0f;
	float PillarPreviewElapsed = 0.0f;
	FVector LastPreviewLocation = FVector::ZeroVector;
	bool bHasPreviewLocation = false;

	float SpawnRingDuration = 0.5f;
};
