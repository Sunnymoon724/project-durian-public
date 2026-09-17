#include "Abilities/IceMakerAbility.h"

#include "Logging/LogMacros.h"

void FIceMakerAbility::Tick(float DeltaTime)
{
	// TODO: 수면·얼음 대상 탐색과 생성 미리보기를 갱신한다.
}

void FIceMakerAbility::HandleInteract()
{
	// TODO: 유효한 수면에 얼음을 생성하거나 기존 얼음을 제거한다.
	UE_LOG(LogTemp, Log, TEXT("IceMaker HandleInteract: ice interaction is not implemented yet."));
}

void FIceMakerAbility::HandleCancel()
{
	// TODO: 얼음 생성 조준 상태와 미리보기를 취소한다.
	UE_LOG(LogTemp, Log, TEXT("IceMaker HandleCancel: ice cancel is not implemented yet."));
}

void FIceMakerAbility::HandleAbilityUse()
{
	// TODO: 얼음 생성 능력의 실행 상태를 시작하거나 종료한다.
	UE_LOG(LogTemp, Log, TEXT("IceMaker HandleAbilityUse: ice ability use is not implemented yet."));
}

void FIceMakerAbility::HandleTargetAtFeet()
{
	// TODO: 얼음 생성 타겟을 플레이어 발밑으로 전환한다.
	UE_LOG(LogTemp, Log, TEXT("IceMaker HandleTargetAtFeet: target-at-feet is not implemented yet."));
}
