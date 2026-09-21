#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KzSceneSubsystem.generated.h"

class UKzScene;
class UKzSceneDefinition;

UCLASS(Config=Game)
class DURIAN_API UKzSceneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool ChangeScene(const UKzSceneDefinition* SceneDefinition);

	UKzScene* GetCurrentScene() const;

	bool IsSceneChanging() const { return bSceneChanging; }

private:
	UFUNCTION()
	void HandlePostLoadMap(UWorld* LoadedWorld);

	bool TryCreateScene(UWorld* World,const UKzSceneDefinition* SceneDefinition, UKzScene*& OutScene);

	UPROPERTY(Transient)
	TObjectPtr<UKzScene> CurrentScene;

	UPROPERTY(Transient)
	TObjectPtr<const UKzSceneDefinition> PendingSceneDefinition;

	bool bSceneChanging = false;
};
