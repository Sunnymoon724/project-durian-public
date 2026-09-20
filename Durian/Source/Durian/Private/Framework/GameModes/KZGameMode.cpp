// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameModes/KzGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Framework/Player/KzPlayerController.h"
#include "GameFramework/Pawn.h"

AKzGameMode::AKzGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerClassFinder(TEXT("/Game/_BP/Characters/BP_PlayerCharacter.BP_PlayerCharacter_C"));

	if (PlayerClassFinder.Succeeded())
	{
		DefaultPawnClass = PlayerClassFinder.Class;
	}

	PlayerControllerClass = AKzPlayerController::StaticClass();
}

void AKzGameMode::BeginPlay()
{
	Super::BeginPlay();
}
