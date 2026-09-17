#pragma once

#include "Abilities/Ability.h"

class FRemoteBombAbility final : public FAbility
{
public:
	explicit FRemoteBombAbility(AMyPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
	void HandleBombThrow();
};
