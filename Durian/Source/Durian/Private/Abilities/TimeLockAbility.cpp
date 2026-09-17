#include "Abilities/TimeLockAbility.h"

#include "Logging/LogMacros.h"

void FTimeLockAbility::Tick(float DeltaTime)
{
	// TODO: 조준 대상 갱신, 정지 타이머, 힘 축적 상태를 처리한다.
}

void FTimeLockAbility::HandleInteract()
{
	// TODO: 조준 중인 물리 오브젝트를 정지시키거나 정지 상태를 적용한다.
	UE_LOG(LogTemp, Log, TEXT("TimeLock HandleInteract: stasis interaction is not implemented yet."));
}

void FTimeLockAbility::HandleCancel()
{
	// TODO: 정지 대상을 해제하고 축적된 힘을 적용한다.
	UE_LOG(LogTemp, Log, TEXT("TimeLock HandleCancel: stasis release is not implemented yet."));
}

void FTimeLockAbility::HandleAbilityUse()
{
	// TODO: 타임록 능력의 사용 가능 상태와 재사용 대기시간을 확인한다.
	UE_LOG(LogTemp, Log, TEXT("TimeLock HandleAbilityUse: stasis use is not implemented yet."));
}
