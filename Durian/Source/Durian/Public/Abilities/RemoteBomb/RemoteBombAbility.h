#pragma once

#include "Abilities/Core/Ability.h"
#include "Abilities/RemoteBomb/RemoteBomb.h"

class FRemoteBombAbility final : public FAbility
{
public:
	explicit FRemoteBombAbility(AKzPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
	void HandleRemoteBombThrow();
	void HandleAbilityDeselected();
	void DropHeldBomb();
	void AbortForEndPlay();
	float GetCooldownRemaining(ERemoteBombShape Shape) const;
	bool HasBomb(ERemoteBombShape Shape) const;
	bool IsBombInstalled(ERemoteBombShape Shape) const;
	bool IsHoldingBomb() const { return HeldBomb.IsValid(); }

private:
	static int32 ToIndex(ERemoteBombShape Shape) { return Shape == ERemoteBombShape::Sphere ? 0 : 1; }
	bool GetSelectedShape(ERemoteBombShape& OutShape) const;
	void SpawnBomb(ERemoteBombShape Shape);
	void DetonateBomb(ERemoteBombShape Shape);
	void ApplyExplosion(ARemoteBomb* Bomb, const FVector& Origin) const;
	FVector FindDropLocation(ARemoteBomb* Bomb) const;
	bool CanPickUp(const ARemoteBomb* Bomb) const;
	void PlaceHeldBomb(bool bThrow);

	TWeakObjectPtr<ARemoteBomb> Bombs[2];
	TWeakObjectPtr<ARemoteBomb> HeldBomb;
	float CooldownRemaining[2] = { 0.0f, 0.0f };
};
