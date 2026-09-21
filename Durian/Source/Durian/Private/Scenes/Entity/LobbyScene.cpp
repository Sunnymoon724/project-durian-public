// Fill out your copyright notice in the Description page of Project Settings.

#include "Scenes/Entity/LobbyScene.h"
#include "UI/KzHudWidget.h"
#include "UI/KzUISubsystem.h"

void ULobbyScene::Initialize(UWorld* World)
{
	Super::Initialize(World);

	if (!World)
	{
		return;
	}

	UKzBaseWidget* Widget = OpenWidget(World,TEXT("WBP_Hud"));

	if (!Widget)
	{
		return;
	}

	HudWidget = Cast<UKzHudWidget>(Widget);
}