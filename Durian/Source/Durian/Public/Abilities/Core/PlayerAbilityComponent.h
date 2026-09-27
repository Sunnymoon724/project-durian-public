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

UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UPlayerAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerAbilityComponent();
	virtual ~UPlayerAbilityComponent() override;
	void SetAbility(EAbilityType NewAbility);
	void HandleInteract();
	void HandleCancel();
	void HandleAbilityUse();
	void HandleDistance(float AxisValue);
	void HandleGuard();
	void HandleAttack(const FVector& AttackDirection);
	void HandleIceTargetAtFeet();
	void HandleRemoteBombThrow();
	void InterruptForDamage();
	void InterruptForStateChange();
	float GetRemoteBombCooldown(ERemoteBombShape Shape) const;
	bool HasRemoteBomb(ERemoteBombShape Shape) const;
	bool IsRemoteBombInstalled(ERemoteBombShape Shape) const;
	bool IsHoldingRemoteBomb() const;
	float GetStasisRemainingTime() const;
	float GetStasisCooldownRemaining() const;
	FVector GetStasisAccumulatedImpulse() const;
	bool IsStasisActive() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombSphereClass;

	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombCubeClass;

	void InterruptTargeting();
	void DropHeldBomb();
	AKzPlayerCharacter* Character = nullptr;
	TUniquePtr<FMagnesisAbility> MagnesisAbility;
	TUniquePtr<FStasisAbility> StasisAbility;
	TUniquePtr<FRemoteBombAbility> RemoteBombAbility;
	TUniquePtr<FCryonisAbility> CryonisAbility;
	FAbility* CurrentAbility = nullptr;
};
