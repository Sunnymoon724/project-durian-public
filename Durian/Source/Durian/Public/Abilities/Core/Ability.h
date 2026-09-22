#pragma once

#include "CoreMinimal.h"

class AKzPlayerCharacter;

class FAbility
{
public:
	explicit FAbility(AKzPlayerCharacter* InCharacter) : Character(InCharacter) { }
	virtual ~FAbility() = default;

	virtual void Tick(float DeltaTime) {}
	virtual void HandleInteract() {}
	virtual void HandleCancel() {}
	virtual void HandleAbilityUse() {}
	virtual void HandleDistance(float AxisValue) {}

protected:
	AKzPlayerCharacter* Character = nullptr;
};
