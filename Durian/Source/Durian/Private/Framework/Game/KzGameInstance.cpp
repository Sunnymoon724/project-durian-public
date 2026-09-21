#include "Framework/Game/KzGameInstance.h"

#include "Framework/Game/KzSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UKzGameInstance::SaveSlotName = TEXT("DurianProgress");

void UKzGameInstance::Init()
{
	Super::Init();

	LoadProgress();
}

bool UKzGameInstance::MarkChallengeCompleted(const FName ChallengeId)
{
	if (ChallengeId.IsNone() || !SaveGame)
	{
		return false;
	}

	if (SaveGame->CompletedChallengeIds.Contains(ChallengeId))
	{
		return true;
	}

	SaveGame->CompletedChallengeIds.Add(ChallengeId);

	return SaveProgress();
}

bool UKzGameInstance::IsChallengeCompleted(const FName ChallengeId) const
{
	return !ChallengeId.IsNone() && SaveGame && SaveGame->CompletedChallengeIds.Contains(ChallengeId);
}

const TArray<FName>& UKzGameInstance::GetCompletedChallengeIds() const
{
	static const TArray<FName> EmptyCompletedChallengeIds;

	return SaveGame ? SaveGame->CompletedChallengeIds : EmptyCompletedChallengeIds;
}

bool UKzGameInstance::SaveProgress()
{
	return SaveGame && UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, SaveUserIndex);
}

void UKzGameInstance::LoadProgress()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		SaveGame = Cast<UKzSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}

	if (!SaveGame)
	{
		SaveGame = Cast<UKzSaveGame>(UGameplayStatics::CreateSaveGameObject(UKzSaveGame::StaticClass()));
	}
}
