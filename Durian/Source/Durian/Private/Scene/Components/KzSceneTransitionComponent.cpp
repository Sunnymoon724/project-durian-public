#include "Scene/Components/KzSceneTransitionComponent.h"

#include "Framework/Game/KzGameInstance.h"
#include "Scene/KzSceneDefinition.h"
#include "Scene/KzSceneSubsystem.h"

UKzSceneTransitionComponent::UKzSceneTransitionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UKzSceneTransitionComponent::Transition()
{
	if (!IsValid(NextSceneDefinition) || !NextSceneDefinition->TargetMap.ToSoftObjectPath().IsValid())
	{
		return false;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;

	if (!GameInstance)
	{
		return false;
	}

	if (!CompletedChallengeId.IsNone())
	{
		UKzGameInstance* KzGameInstance = Cast<UKzGameInstance>(GameInstance);

		if (!KzGameInstance)
		{
			return false;
		}

		KzGameInstance->MarkChallengeCompleted(CompletedChallengeId);
	}

	UKzSceneSubsystem* SceneSubsystem = GameInstance->GetSubsystem<UKzSceneSubsystem>();

	return SceneSubsystem && SceneSubsystem->ChangeScene(NextSceneDefinition);
}
