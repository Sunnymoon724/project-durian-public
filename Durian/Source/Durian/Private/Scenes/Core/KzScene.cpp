#include "Scenes/Core/KZScene.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UI/KzBaseWidget.h"
#include "UI/KzUISubsystem.h"

void UKzScene::Initialize(UWorld* World)
{
	
}

void UKzScene::Deinitialize()
{

}

UKzBaseWidget* UKzScene::OpenWidget(UWorld* World, const FString& WidgetName)
{
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		return nullptr;
	}
	
	UKzUISubsystem* UISubsystem = GameInstance->GetSubsystem<UKzUISubsystem>();

	if (!UISubsystem || WidgetName.IsEmpty())
	{
		return nullptr;
	}

	const FString WidgetPath = FString::Printf(TEXT("/Game/_BP/Widgets/%s.%s_C"), *WidgetName, *WidgetName);

	const TSoftClassPtr<UKzBaseWidget> WidgetClass{FSoftObjectPath(WidgetPath)};
	const TSubclassOf<UKzBaseWidget> LoadedWidgetClass = WidgetClass.LoadSynchronous();

	if (!LoadedWidgetClass)
	{
		return nullptr;
	}

	return UISubsystem->Open(LoadedWidgetClass);
}
