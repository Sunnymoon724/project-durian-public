#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnvironmentColorDataAsset.generated.h"

UCLASS(BlueprintType)
class DURIAN_API UEnvironmentColorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment Colors")
	TMap<FString, FLinearColor> Magnesis;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment Colors")
	TMap<FString, FLinearColor> Cryonis;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment Colors")
	TMap<FString, FLinearColor> Stasis;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment Colors")
	TMap<FString, FLinearColor> RemoteBomb;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment Colors")
	TMap<FString, FLinearColor> Lobby;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment Colors")
	TMap<FString, FLinearColor> DungeonStatus;
};
