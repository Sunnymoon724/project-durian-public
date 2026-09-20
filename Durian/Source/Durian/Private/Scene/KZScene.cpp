#include "Scene/KzScene.h"
#include "Scene/KzSceneDefinition.h"

bool UKzScene::Initialize(const UKzSceneDefinition* InDefinition)
{
	Definition = InDefinition;

	return IsValid(Definition);
}

void UKzScene::Deinitialize()
{
	Definition = nullptr;
}
