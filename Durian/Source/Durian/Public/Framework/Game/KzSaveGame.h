#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "KzSaveGame.generated.h"

UCLASS()
class DURIAN_API UKzSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, SaveGame, Category="Progress")
	TArray<FName> CompletedChallengeIds;
};
