#pragma once

#include "Abilities/Core/Ability.h"
#include "Abilities/RemoteBomb/RemoteBomb.h"
#include "RemoteBombAbility.generated.h"

UCLASS(BlueprintType)
class DURIAN_API URemoteBombAbility final : public UAbility
{
	GENERATED_BODY()

public:
	void SetBombClasses(TSubclassOf<ARemoteBomb> InSphereClass, TSubclassOf<ARemoteBomb> InCubeClass);

	void Tick(float DeltaTime) override;
	void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) override;
	void OnDeselected() override;
	void DropHeldBomb();
	void AbortForEndPlay();
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetCooldownRemaining(ERemoteBombShape Shape) const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasBomb(ERemoteBombShape Shape) const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsBombInstalled(ERemoteBombShape Shape) const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
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

	double CooldownEndTimeArray[2] = { 0.0, 0.0 };
};
