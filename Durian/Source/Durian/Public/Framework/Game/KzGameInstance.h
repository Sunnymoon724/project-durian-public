#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KzGameInstance.generated.h"

class UKzSaveGame;

UCLASS()
class DURIAN_API UKzGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category="Progress")
	bool MarkChallengeCompleted(FName ChallengeId);

	UFUNCTION(BlueprintPure, Category="Progress")
	bool IsChallengeCompleted(FName ChallengeId) const;

	const TArray<FName>& GetCompletedChallengeIds() const;

	UFUNCTION(BlueprintCallable, Category="Progress")
	bool SaveProgress();

private:
	void LoadProgress();

	UPROPERTY(Transient)
	TObjectPtr<UKzSaveGame> SaveGame;

	static const FString SaveSlotName;
	static constexpr int32 SaveUserIndex = 0;
};
