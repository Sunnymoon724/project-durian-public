#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/Cryonis/CryonisAbility.h"
#include "Abilities/Magnesis/MagnesisAbility.h"
#include "Abilities/RemoteBomb/RemoteBombAbility.h"
#include "Abilities/Stasis/StasisAbility.h"
#include "Enums/PlayerEnums.h"
#include "PlayerAbilityComponent.generated.h"

class AKzPlayerCharacter;
class ARemoteBomb;

UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UPlayerAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerAbilityComponent();

	void SetAbility(EAbilityType NewAbility);
	void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) const;
	void HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection) const;

	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsHoldingRemoteBomb() const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetRemoteBombSphereCooldown() const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetRemoteBombCubeCooldown() const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasRemoteBombSphere() const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasRemoteBombCube() const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsRemoteBombSphereInstalled() const;
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsRemoteBombCubeInstalled() const;

	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetStasisRemainingTime() const;
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetStasisCooldownRemaining() const;
	UFUNCTION(BlueprintPure, Category = "Stasis")
	FVector GetStasisAccumulatedImpulse() const;
	UFUNCTION(BlueprintPure, Category = "Stasis")
	bool IsStasisActive() const;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float GetRemoteBombCooldown(ERemoteBombShape Shape) const;
	bool HasRemoteBomb(ERemoteBombShape Shape) const;
	bool IsRemoteBombInstalled(ERemoteBombShape Shape) const;

	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombSphereClass;

	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombCubeClass;

	UPROPERTY(Transient)
	TObjectPtr<AKzPlayerCharacter> Character = nullptr;

	TUniquePtr<FMagnesisAbility> MagnesisAbility;
	TUniquePtr<FStasisAbility> StasisAbility;
	TUniquePtr<FRemoteBombAbility> RemoteBombAbility;
	TUniquePtr<FCryonisAbility> CryonisAbility;

	FAbility* CurrentAbility = nullptr;
};
