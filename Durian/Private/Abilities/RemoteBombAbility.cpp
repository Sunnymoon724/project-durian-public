#include "Abilities/RemoteBombAbility.h"

#include "Logging/LogMacros.h"

void FRemoteBombAbility::Tick(float DeltaTime)
{
	// TODO: 폭탄 미리보기와 형태별 상태·재사용 대기시간을 갱신한다.
}

void FRemoteBombAbility::HandleInteract()
{
	// TODO: 폭탄을 내려놓거나 설치된 폭탄을 줍는다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleInteract: bomb interaction is not implemented yet."));
}

void FRemoteBombAbility::HandleCancel()
{
	// TODO: 폭탄 생성을 취소하되 이미 설치된 폭탄은 유지한다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleCancel: bomb cancel is not implemented yet."));
}

void FRemoteBombAbility::HandleAbilityUse()
{
	// TODO: 폭탄을 생성하거나 이미 생성된 폭탄을 기폭한다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleAbilityUse: bomb spawn/detonate is not implemented yet."));
}

void FRemoteBombAbility::HandleBombThrow()
{
	// TODO: 보유 중인 폭탄을 던진다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleBombThrow: bomb throw is not implemented yet."));
}
