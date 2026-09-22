// Fill out your copyright notice in the Description page of Project Settings.

#include "Scenes/Entity/LobbyScene.h"
#include "UI/KzUISubsystem.h"
#include "UI/Widget/KzHudWidget.h"
#include "Engine/GameInstance.h"

void ULobbyScene::Initialize(UWorld* World)
{
	Super::Initialize(World);

	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	UKzUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UKzUISubsystem>() : nullptr;

	if (!UISubsystem)
	{
		return;
	}

	UISubsystem->Open<UKzHudWidget>();
}
