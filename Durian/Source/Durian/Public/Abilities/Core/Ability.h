#pragma once

#include "CoreMinimal.h"

class AKzPlayerCharacter;
class AActor;
class UPrimitiveComponent;
struct FHitResult;

enum class EAbilityReactionType : uint8;

class FAbility
{
public:
	explicit FAbility(AKzPlayerCharacter* InCharacter) : Character(InCharacter) { }
	virtual ~FAbility() = default;

	virtual void Tick(float DeltaTime) {}
	virtual void HandleInteract() {}
	virtual void HandleCancel() {}
	virtual void HandleAbilityUse() {}
	virtual void HandleDistance(float AxisValue) {}

protected:
	bool TraceAbilityTarget(EAbilityReactionType ReactionType, float TraceRange, FHitResult& OutHit, bool bTargetAtFeet = false, const AActor* IgnoredActor = nullptr, bool bRequirePhysics = false) const;

	AKzPlayerCharacter* Character = nullptr;
};
