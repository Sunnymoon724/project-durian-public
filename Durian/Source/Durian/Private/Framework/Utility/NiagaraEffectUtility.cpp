#include "Framework/Utility/NiagaraEffectUtility.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponentPoolMethodEnum.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

UNiagaraComponent* FNiagaraEffectUtility::SpawnAtLocation(UWorld* World, const FSoftObjectPath& SystemPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
	if (!World || SystemPath.IsNull())
	{
		return nullptr;
	}

	UNiagaraSystem* System = Cast<UNiagaraSystem>(SystemPath.TryLoad());
	if (!System)
	{
		return nullptr;
	}

	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, Location, Rotation, Scale, true, true, ENCPoolMethod::AutoRelease, true);
}

UNiagaraComponent* FNiagaraEffectUtility::SpawnAtLocation(UWorld* World, const TSoftObjectPtr<UNiagaraSystem>& System, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
	return SpawnAtLocation(World, System.ToSoftObjectPath(), Location, Rotation, Scale);
}

UNiagaraComponent* FNiagaraEffectUtility::SpawnAttachedRelative(USceneComponent* AttachToComponent, const FSoftObjectPath& SystemPath, const FName SocketName, const FVector& Location, const FRotator& Rotation, const bool bAutoDestroy, const bool bAutoActivate)
{
	if (!IsValid(AttachToComponent) || SystemPath.IsNull())
	{
		return nullptr;
	}

	UNiagaraSystem* System = Cast<UNiagaraSystem>(SystemPath.TryLoad());
	if (!System)
	{
		return nullptr;
	}

	return UNiagaraFunctionLibrary::SpawnSystemAttached(System, AttachToComponent, SocketName, Location, Rotation, EAttachLocation::KeepRelativeOffset, bAutoDestroy, bAutoActivate);
}

UNiagaraComponent* FNiagaraEffectUtility::SpawnAttachedRelative(USceneComponent* AttachToComponent, const TSoftObjectPtr<UNiagaraSystem>& System, const FName SocketName, const FVector& Location, const FRotator& Rotation, const bool bAutoDestroy, const bool bAutoActivate)
{
	return SpawnAttachedRelative(AttachToComponent, System.ToSoftObjectPath(), SocketName, Location, Rotation, bAutoDestroy, bAutoActivate);
}
