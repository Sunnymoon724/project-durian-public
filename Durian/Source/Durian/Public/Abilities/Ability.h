#pragma once

#include "CoreMinimal.h"

class AMyPlayerCharacter;

class FAbility
{
public:
	explicit FAbility(AMyPlayerCharacter* InCharacter)
		: Character(InCharacter)
	{
	}

	virtual ~FAbility() = default;

	virtual void Tick(float DeltaTime) {}
	virtual void HandleInteract() {}
	virtual void HandleCancel() {}
	virtual void HandleAbilityUse() {}
	virtual void HandleDistance(float AxisValue) {}

protected:
	AMyPlayerCharacter* Character = nullptr;
};
