#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageableComponent.generated.h"

class AController;
class UDamageType;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDamageableHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDamageableDepleted);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class DURIAN_API UDamageableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageableComponent();
	UFUNCTION(BlueprintPure, Category = "Damage")
	float GetHealth() const { return Health; }
	UFUNCTION(BlueprintPure, Category = "Damage")
	float GetMaxHealth() const { return MaxHealth; }
	void SetDestroyOwnerOnDepleted(bool bDestroy) { bDestroyOwnerOnDepleted = bDestroy; }

	UPROPERTY(BlueprintAssignable, Category = "Damage")
	FDamageableHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Damage")
	FDamageableDepleted OnDepleted;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (ClampMin = "0.01"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Damage")
	bool bIndestructible = false;

	UPROPERTY(EditAnywhere, Category = "Damage")
	bool bDestroyOwnerOnDepleted = true;

	UPROPERTY(VisibleInstanceOnly, Category = "Damage")
	float Health = 100.0f;
};
