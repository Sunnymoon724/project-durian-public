#include "Scenes/Components/KzSceneTransitionComponent.h"

#include "Framework/Game/KzGameInstance.h"
#include "Scenes/Core/KZSceneDefinition.h"
#include "Scenes/Core/KzSceneSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

UKzSceneTransitionComponent::UKzSceneTransitionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UKzSceneTransitionComponent::Transition()
{
	if (bTransitionPending || !IsValid(NextSceneDefinition) || !NextSceneDefinition->TargetMap.ToSoftObjectPath().IsValid())
	{
		return false;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;

	if (!GameInstance)
	{
		return false;
	}

	if (!CompletedChallengeId.IsNone() && !Cast<UKzGameInstance>(GameInstance))
	{
		return false;
	}
	const TCHAR* EffectPath = CompletedChallengeId.IsNone() ? TEXT("/Game/Resources/VFX/Challenge/Niagara/NS_ChallengeEnter.NS_ChallengeEnter") : TEXT("/Game/Resources/VFX/Challenge/Niagara/NS_ChallengeReturn.NS_ChallengeReturn");
	if (UNiagaraSystem* Effect = LoadObject<UNiagaraSystem>(nullptr, EffectPath))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, GetOwner()->GetActorLocation());
	}
	if (!CompletedChallengeId.IsNone())
	{
		if (UNiagaraSystem* ClearEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Challenge/Niagara/NS_ChallengeClear.NS_ChallengeClear")))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ClearEffect, GetOwner()->GetActorLocation());
		}
	}
	bTransitionPending = true;
	GetWorld()->GetTimerManager().SetTimer(TransitionTimer, this, &UKzSceneTransitionComponent::FinishTransition, 0.6f, false);
	return true;
}

void UKzSceneTransitionComponent::FinishTransition()
{
	bTransitionPending = false;
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GameInstance || !IsValid(NextSceneDefinition))
	{
		return;
	}
	if (!CompletedChallengeId.IsNone())
	{
		CastChecked<UKzGameInstance>(GameInstance)->MarkChallengeCompleted(CompletedChallengeId);
	}
	if (UKzSceneSubsystem* SceneSubsystem = GameInstance->GetSubsystem<UKzSceneSubsystem>())
	{
		SceneSubsystem->ChangeScene(NextSceneDefinition);
	}
}
