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
class AActor;

UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UPlayerAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerAbilityComponent();

	void SetAbility(EAbilityType NewAbility);
	UAbility* GetAbility(EAbilityType AbilityType) const;

	void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) const;
	void HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection) const;

	AActor* GetMagnesisHeldActor() const { return MagnesisHeldActor.Get(); }
	void SetMagnesisHeldActor(AActor* Actor);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombSphereClass;

	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombCubeClass;

	UPROPERTY(Transient)
	TObjectPtr<AKzPlayerCharacter> Character = nullptr;
	TWeakObjectPtr<AActor> MagnesisHeldActor;

	UPROPERTY(Transient)
	TObjectPtr<UMagnesisAbility> MagnesisAbility;
	UPROPERTY(Transient)
	TObjectPtr<UStasisAbility> StasisAbility;
	UPROPERTY(Transient)
	TObjectPtr<URemoteBombAbility> RemoteBombAbility;
	UPROPERTY(Transient)
	TObjectPtr<UCryonisAbility> CryonisAbility;

	UPROPERTY(Transient)
	TObjectPtr<UAbility> CurrentAbility;
};
