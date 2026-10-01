#pragma once

#include "Abilities/Core/Ability.h"
#include "Abilities/RemoteBomb/RemoteBomb.h"

class FRemoteBombAbility final : public FAbility
{
public:
	FRemoteBombAbility(AKzPlayerCharacter* InCharacter, TSubclassOf<ARemoteBomb> InSphereClass, TSubclassOf<ARemoteBomb> InCubeClass) : FAbility(InCharacter), BombClassArray{ InSphereClass, InCubeClass } { }

	void Tick(float DeltaTime) override;
	void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) override;
	void HandleAbilityDeselected();
	void DropHeldBomb();
	void AbortForEndPlay();
	float GetCooldownRemaining(ERemoteBombShape Shape) const;
	bool HasBomb(ERemoteBombShape Shape) const;
	bool IsBombInstalled(ERemoteBombShape Shape) const;
	bool IsHoldingBomb() const { return HeldBomb.IsValid(); }

private:
	static int32 ToIndex(const ERemoteBombShape Shape) { return Shape == ERemoteBombShape::Sphere ? 0 : 1; }
	bool GetSelectedShape(ERemoteBombShape& OutShape) const;
	void SpawnBomb(ERemoteBombShape Shape);
	void DetonateBomb(ERemoteBombShape Shape);
	void ApplyExplosion(ARemoteBomb* Bomb, const FVector& Origin, float Radius) const;
	FVector FindDropLocation(const ARemoteBomb* Bomb) const;
	bool CanPickUp(const ARemoteBomb* Bomb) const;
	void PlaceHeldBomb(bool bThrow);

	TWeakObjectPtr<ARemoteBomb> BombArray[2];
	TSubclassOf<ARemoteBomb> BombClassArray[2];

	TWeakObjectPtr<ARemoteBomb> HeldBomb;
	bool bThrowPending = false;
	float ThrowElapsed = 0.0f;

	double CooldownEndTimeArray[2] = { 0.0, 0.0 };
};
