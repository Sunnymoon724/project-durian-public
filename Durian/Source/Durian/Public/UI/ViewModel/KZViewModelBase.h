// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "KzViewModelBase.generated.h"

/**
 * 
 */
UCLASS()
class DURIAN_API UKzViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize();
};
