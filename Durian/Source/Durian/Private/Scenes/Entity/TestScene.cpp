// Fill out your copyright notice in the Description page of Project Settings.


#include "Scenes/Entity/TestScene.h"
#include "UI/KzUISubsystem.h"
#include "UI/Widget/KzHudWidget.h"
#include "Engine/GameInstance.h"

void UTestScene::Initialize(UWorld* World)
{
	Super::Initialize(World);
	
	if (!World)
	{
		return;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	UKzUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UKzUISubsystem>() : nullptr;

	if (!UISubsystem)
	{
		return;
	}

	UISubsystem->Open<UKzHudWidget>();
}
