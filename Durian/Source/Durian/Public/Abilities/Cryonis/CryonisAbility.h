#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/Ability.h"

class AActor;
class AIcePillar;
class AIcePlacementPreview;

class FCryonisAbility final : public FAbility
{
public:
	explicit FCryonisAbility(AKzPlayerCharacter* InCharacter) : FAbility(InCharacter) { }

	virtual void Tick(float DeltaTime) override;
	virtual void HandleInteract() override;
	virtual void HandleCancel() override;
	virtual void HandleAbilityUse() override;

	void HandleTargetAtFeet();

private:
	void UpdateTargeting();
	void SpawnIcePillar();
	void RemoveTargetedPillar();
	void ExitTargetingMode();
	void ClearTarget();
	void UpdateAimedTarget(AActor* NewTarget);
	void EnsurePreview();
	static bool IsIcePillarTarget(const AActor* Actor);
	bool TraceTarget(FHitResult& OutHit) const;
	bool CanSpawnAt(const FVector& SpawnLocation,const AActor* SurfaceActor) const;
	static FVector GetSpawnLocation(const FHitResult& Hit);
	static void DestroyPillar(AIcePillar* Pillar, bool bShatter = false);

	TWeakObjectPtr<AActor> TargetSurface;
	TWeakObjectPtr<AIcePillar> TargetPillar;
	TWeakObjectPtr<AActor> AimedTargetActor;
	TWeakObjectPtr<AIcePlacementPreview> PlacementPreview;
	TArray<TWeakObjectPtr<AIcePillar>> SpawnedPillars;

	FVector TargetLocation = FVector::ZeroVector;
	float SpawnCooldownRemaining = 0.0f;
	bool bTargetAtFeet = false;
	bool bTargetValid = false;

};
