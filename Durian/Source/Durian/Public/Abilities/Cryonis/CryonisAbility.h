#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/Ability.h"

class AActor;
class AIcePillar;
class AIcePlacementPreview;

class FCryonisAbility final : public FAbility
{
public:
	explicit FCryonisAbility(AMyPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
	void HandleTargetAtFeet();

private:
	void UpdateTargeting();
	void SpawnIcePillar();
	void RemoveTargetedPillar();
	void ExitTargetingMode();
	void ClearTarget();
	void EnsurePreview();
	bool TraceTarget(FHitResult& OutHit) const;
	bool CanSpawnAt(const FVector& SpawnLocation, AActor* SurfaceActor) const;
	FVector GetSpawnLocation(const FHitResult& Hit) const;
	void DestroyPillar(AIcePillar* Pillar);

	TWeakObjectPtr<AActor> TargetSurface;
	TWeakObjectPtr<AIcePillar> TargetPillar;
	TWeakObjectPtr<AIcePlacementPreview> PlacementPreview;
	TArray<TWeakObjectPtr<AIcePillar>> SpawnedPillars;

	FVector TargetLocation = FVector::ZeroVector;
	float SpawnCooldownRemaining = 0.0f;
	bool bTargetAtFeet = false;
	bool bTargetValid = false;

	static constexpr int32 MaxPillarCount = 3;
	static constexpr float SpawnCooldown = 2.0f;
	static constexpr float TargetRange = 1500.0f;
};
