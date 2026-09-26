#include "Abilities/Components/DamageableComponent.h"

#include "GameFramework/Actor.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

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
	if (IsValid(DamagedActor) && !DamagedActor->IsA<AKzPlayerCharacter>())
	{
		const TCHAR* EffectPath = Health > 0.0f ? TEXT("/Game/Resources/VFX/Combat/Niagara/NS_SentinelHit.NS_SentinelHit") : TEXT("/Game/Resources/VFX/Combat/Niagara/NS_SentinelDeath.NS_SentinelDeath");
		if (UNiagaraSystem* Effect = LoadObject<UNiagaraSystem>(nullptr, EffectPath))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, DamagedActor->GetActorLocation());
		}
	}
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
