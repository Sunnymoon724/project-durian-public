#pragma once

#include "Abilities/Ability.h"

class FIceMakerAbility final : public FAbility
{
public:
	explicit FIceMakerAbility(AMyPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
	void HandleTargetAtFeet();
};
