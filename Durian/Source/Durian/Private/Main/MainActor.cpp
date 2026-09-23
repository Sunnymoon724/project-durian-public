// Fill out your copyright notice in the Description page of Project Settings.


#include "Main/MainActor.h"
#include "Main/MainConfig.h"
#include "Scenes/Core/KZSceneDefinition.h"
#include "Scenes/Core/KzSceneSubsystem.h"

// Sets default values
AMainActor::AMainActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Called when the game starts or when spawned
void AMainActor::BeginPlay()
{
	Super::BeginPlay();

	if (!MainConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("MainConfig is not assigned."));

		return;
	}

	UKzSceneSubsystem* SceneSubsystem = GetGameInstance()->GetSubsystem<UKzSceneSubsystem>();

	if (!SceneSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("KzSceneSubsystem is unavailable."));

		return;
	}
	
	const UKzSceneDefinition* SceneDefinition = MainConfig->GetStartupScene().LoadSynchronous();

	SceneSubsystem->ChangeScene(SceneDefinition);
}
