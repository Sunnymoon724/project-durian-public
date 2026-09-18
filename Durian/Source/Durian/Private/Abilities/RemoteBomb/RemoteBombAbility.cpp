#include "Abilities/RemoteBomb/RemoteBombAbility.h"

void FRemoteBombAbility::Tick(float DeltaTime)
{
	// TODO: 리모트 폭탄 미리보기와 형태별 상태·재사용 대기시간을 갱신한다.
}

void FRemoteBombAbility::HandleInteract()
{
	// TODO: 리모트 폭탄을 내려놓거나 설치된 리모트 폭탄을 줍는다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleInteract: remote bomb interaction is not implemented yet."));
}

void FRemoteBombAbility::HandleCancel()
{
	// TODO: 리모트 폭탄 생성을 취소하되 이미 설치된 리모트 폭탄은 유지한다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleCancel: remote bomb cancel is not implemented yet."));
}

void FRemoteBombAbility::HandleAbilityUse()
{
	// TODO: 리모트 폭탄을 생성하거나 이미 생성된 리모트 폭탄을 기폭한다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleAbilityUse: remote bomb spawn/detonate is not implemented yet."));
}

void FRemoteBombAbility::HandleRemoteBombThrow()
{
	// TODO: 보유 중인 리모트 폭탄을 던진다.
	UE_LOG(LogTemp, Log, TEXT("RemoteBomb HandleRemoteBombThrow: remote bomb throw is not implemented yet."));
}
