#include "Abilities/Core/Ability.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/Controller.h"

bool FAbility::TraceAbilityTarget(const EAbilityReactionType ReactionType, const float TraceRange, FHitResult& OutHit, const bool bTargetAtFeet, const AActor* IgnoredActor, const bool bRequirePhysics) const
{
	if (!Character || !Character->GetWorld() || !Character->GetController())
	{
		return false;
	}

	FVector TraceStart = Character->GetPawnViewLocation();
	FVector TraceEnd = TraceStart + Character->GetController()->GetControlRotation().Vector() * TraceRange;

	if (bTargetAtFeet)
	{
		TraceStart = Character->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		TraceEnd = Character->GetActorLocation() - FVector(0.0f, 0.0f, 200.0f);
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AbilityTargetTrace), false, Character);

	QueryParams.AddIgnoredActor(Character);

	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	if (!Character->GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	const AActor* HitActor = OutHit.GetActor();
	const UAbilityReactionComponent* Reaction = HitActor ? HitActor->FindComponentByClass<UAbilityReactionComponent>() : nullptr;

	const UPrimitiveComponent* HitComponent = OutHit.GetComponent();

	if (!Reaction || Reaction->GetReactionType() != ReactionType || !HitComponent || (bRequirePhysics && !HitComponent->IsSimulatingPhysics()))
	{
		return false;
	}

	return true;
}
