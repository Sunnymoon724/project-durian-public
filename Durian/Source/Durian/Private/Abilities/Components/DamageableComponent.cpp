#include "Abilities/Components/DamageableComponent.h"

#include "GameFramework/Actor.h"

UDamageableComponent::UDamageableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDamageableComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UDamageableComponent::HandleAnyDamage);
	}
}

void UDamageableComponent::HandleAnyDamage(AActor* DamagedActor, const float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || Health <= 0.0f || bIndestructible)
	{
		return;
	}

	Health = FMath::Max(0.0f, Health - Damage);
	OnHealthChanged.Broadcast(Health, MaxHealth);
	if (Health > 0.0f)
	{
		return;
	}

	OnDepleted.Broadcast();
	if (bDestroyOwnerOnDepleted && IsValid(DamagedActor))
	{
		DamagedActor->Destroy();
	}
}
