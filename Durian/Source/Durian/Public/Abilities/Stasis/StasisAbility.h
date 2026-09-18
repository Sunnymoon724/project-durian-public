#pragma once

#include "Abilities/Core/Ability.h"

class FStasisAbility final : public FAbility
{
public:
	explicit FStasisAbility(AMyPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
};
