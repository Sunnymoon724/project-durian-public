#pragma once

#include "Abilities/Ability.h"

class FTimeLockAbility final : public FAbility
{
public:
	explicit FTimeLockAbility(AMyPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
};
