#include "Scene/KzSceneSubsystem.h"
#include "Scene/KzScene.h"
#include "Scene/KzSceneDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

void UKzSceneSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UKzSceneSubsystem::HandlePostLoadMap);
}

void UKzSceneSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	if (IsValid(CurrentScene))
	{
		CurrentScene->Deinitialize();
	}

	CurrentScene = nullptr;

	PendingSceneDefinition = nullptr;
	bSceneChanging = false;

	Super::Deinitialize();
}

bool UKzSceneSubsystem::ChangeScene(const UKzSceneDefinition* SceneDefinition)
{
	if (bSceneChanging || !IsValid(SceneDefinition) || !SceneDefinition->TargetMap.ToSoftObjectPath().IsValid())
	{
		return false;
	}
	
	bSceneChanging = true;

	if (IsValid(CurrentScene))
	{
		CurrentScene->Deinitialize();
	}

	CurrentScene = nullptr;
	PendingSceneDefinition = SceneDefinition;

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, SceneDefinition->TargetMap);

	return true;
}

UKzScene* UKzSceneSubsystem::GetCurrentScene() const
{
	return CurrentScene;
}

void UKzSceneSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!bSceneChanging || !PendingSceneDefinition)
	{
		return;
	}

	UKzScene* Scene = nullptr;

	if (TryCreateScene(PendingSceneDefinition, Scene))
	{
		CurrentScene = Scene;
	}

	PendingSceneDefinition = nullptr;
	bSceneChanging = false;
}

bool UKzSceneSubsystem::TryCreateScene(const UKzSceneDefinition* SceneDefinition, UKzScene*& OutScene)
{
	OutScene = nullptr;

	if (!IsValid(SceneDefinition) || !SceneDefinition->SceneClass)
	{
		return false;
	}

	UKzScene* Scene = NewObject<UKzScene>(this, SceneDefinition->SceneClass);

	if (!Scene || !Scene->Initialize(SceneDefinition))
	{
		return false;
	}

	OutScene = Scene;

	return true;
}
