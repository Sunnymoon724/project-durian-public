// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Pawn.h"

AMyGameMode::AMyGameMode()
{
    static ConstructorHelpers::FClassFinder<APawn> PlayerClassFinder(TEXT("/Game/_BP/Characters/BP_PlayerCharacter.BP_PlayerCharacter_C"));

    if (PlayerClassFinder.Succeeded())
    {
        DefaultPawnClass = PlayerClassFinder.Class;
    }
}
