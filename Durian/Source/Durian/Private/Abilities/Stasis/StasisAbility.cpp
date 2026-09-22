#include "Abilities/Stasis/StasisAbility.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Constants/GameConstants.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"

void FStasisAbility::Tick(float DeltaTime)
{
	if (!Character) return;
	CooldownRemaining = FMath::Max(0.0f, CooldownRemaining - DeltaTime);

	switch (Character->GetCurrentState())
	{
	case EPlayerState::StasisTargeting: UpdateTargeting(); break;
	case EPlayerState::StasisActive:
		RemainingTime -= DeltaTime;
		if (RemainingTime <= 0.0f) EndStasis(true);
		break;
	default: break;
	}
}

void FStasisAbility::HandleInteract()
{
	if (Character && Character->GetCurrentState() == EPlayerState::StasisTargeting) StartStasis();
}

void FStasisAbility::HandleCancel()
{
	if (Character && Character->GetCurrentState() == EPlayerState::StasisTargeting)
	{
		ClearTarget();
		Character->SetPlayerState(EPlayerState::Normal);
		if (UGameInstance* GI = Character->GetGameInstance()) GI->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Stasis, false);
		if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect()) Effect->ClearVisionEffects();
	}
	else if (Character && Character->GetCurrentState() == EPlayerState::StasisActive) EndStasis(false);
}

void FStasisAbility::HandleAbilityUse()
{
	if (!Character || CooldownRemaining > 0.0f) return;
	if (Character->GetCurrentState() == EPlayerState::Normal)
	{
		Character->SetPlayerState(EPlayerState::StasisTargeting);
		if (UGameInstance* GI = Character->GetGameInstance()) GI->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Stasis, true);
		if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect()) Effect->SetVisionEnabled(EAbilityType::Stasis, true);
	}
	else if (Character->GetCurrentState() == EPlayerState::StasisTargeting) HandleCancel();
}

void FStasisAbility::AccumulateImpulse(const FVector& Impulse)
{
	if (!Character || Character->GetCurrentState() != EPlayerState::StasisActive) return;
	AccumulatedImpulse = (AccumulatedImpulse + Impulse).GetClampedToMaxSize(Constants::StasisMaxImpulse);
}

void FStasisAbility::UpdateTargeting()
{
	// 조준 중에는 타임록 월드 스캔 효과를 유지한다.
	if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect())
	{
		Effect->SetVisionEnabled(EAbilityType::Stasis, true);
	}

	UPrimitiveComponent* NewTarget = nullptr;
	if (TraceTarget(NewTarget))
	{
		if (TargetedComponent.Get() != NewTarget) ClearTarget();
		TargetedComponent = NewTarget;
		NewTarget->SetRenderCustomDepth(true);
		NewTarget->SetCustomDepthStencilValue(2);
		if (UAbilityReactionComponent* Reaction = NewTarget->GetOwner()->FindComponentByClass<UAbilityReactionComponent>()) Reaction->SetAimedTarget(true);
	}
	else ClearTarget();
}

void FStasisAbility::StartStasis()
{
	UPrimitiveComponent* Target = nullptr;
	if (!TraceTarget(Target) || !Target->IsSimulatingPhysics()) { HandleCancel(); return; }
	TargetedComponent = Target;
	SavedLinearVelocity = Target->GetPhysicsLinearVelocity();
	SavedAngularVelocity = Target->GetPhysicsAngularVelocityInRadians();
	AccumulatedImpulse = FVector::ZeroVector;
	Target->SetSimulatePhysics(false);
	RemainingTime = Constants::StasisDuration;
	Character->SetPlayerState(EPlayerState::StasisActive);
	UE_LOG(LogTemp, Log, TEXT("Stasis started on %s"), *GetNameSafe(Target->GetOwner()));
}

void FStasisAbility::EndStasis(const bool bApplyImpulse)
{
	if (TargetedComponent.IsValid())
	{
		TargetedComponent->SetSimulatePhysics(true);
		TargetedComponent->SetPhysicsLinearVelocity(SavedLinearVelocity);
		TargetedComponent->SetPhysicsAngularVelocityInRadians(SavedAngularVelocity);
		if (bApplyImpulse && !AccumulatedImpulse.IsNearlyZero()) TargetedComponent->AddImpulse(AccumulatedImpulse);
	}
	ClearTarget();
	AccumulatedImpulse = FVector::ZeroVector;
	CooldownRemaining = Constants::StasisCooldown;
	Character->SetPlayerState(EPlayerState::Normal);
	if (UGameInstance* GI = Character->GetGameInstance()) GI->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Stasis, false);
	if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect()) Effect->ClearVisionEffects();
}

void FStasisAbility::ClearTarget()
{
	if (TargetedComponent.IsValid())
	{
		TargetedComponent->SetCustomDepthStencilValue(1);
		if (UAbilityReactionComponent* Reaction = TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>()) Reaction->SetAimedTarget(false);
	}
	TargetedComponent.Reset();
}

bool FStasisAbility::TraceTarget(UPrimitiveComponent*& OutComponent) const
{
	OutComponent = nullptr;
	if (!Character || !Character->GetController() || !Character->GetWorld()) return false;
	const FVector Start = Character->GetPawnViewLocation();
	const FVector End = Start + Character->GetController()->GetControlRotation().Vector() * Constants::StasisTargetRange;
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(StasisTargetTrace), false, Character);
	Params.AddIgnoredActor(Character);
	const bool bHit = Character->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	UPrimitiveComponent* Component = Hit.GetComponent();
	const UAbilityReactionComponent* Reaction = Hit.GetActor() ? Hit.GetActor()->FindComponentByClass<UAbilityReactionComponent>() : nullptr;
	const bool bValidTarget = bHit && Component && Component->IsSimulatingPhysics() && Reaction && Reaction->GetReactionType() == EAbilityReactionType::StasisTarget;
	if (!bValidTarget) return false;
	OutComponent = Component;
	return true;
}
