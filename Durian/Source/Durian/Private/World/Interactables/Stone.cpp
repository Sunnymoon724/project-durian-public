#include "World/Interactables/Stone.h"

#include "Abilities/RemoteBomb/RemoteBomb.h"

AStone::AStone()
{
	PrimaryActorTick.bCanEverTick = false;

	SetCanBeDamaged(true);
}

float AStone::TakeDamage(const float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (bIsBroken || !Cast<ARemoteBomb>(DamageCauser))
	{
		return AppliedDamage;
	}

	bIsBroken = true;

	Destroy();

	return AppliedDamage;
}
