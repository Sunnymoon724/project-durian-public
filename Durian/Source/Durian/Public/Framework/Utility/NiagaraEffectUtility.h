#pragma once

#include "CoreMinimal.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UWorld;

class DURIAN_API FNiagaraEffectUtility final
{
public:
	static UNiagaraComponent* SpawnAtLocation(UWorld* World, const FSoftObjectPath& SystemPath, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector::OneVector);
	static UNiagaraComponent* SpawnAtLocation(UWorld* World, const TSoftObjectPtr<UNiagaraSystem>& System, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector::OneVector);
	static UNiagaraComponent* SpawnAttachedRelative(USceneComponent* AttachToComponent, const FSoftObjectPath& SystemPath, FName SocketName = NAME_None, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, bool bAutoDestroy = true, bool bAutoActivate = true);
	static UNiagaraComponent* SpawnAttachedRelative(USceneComponent* AttachToComponent, const TSoftObjectPtr<UNiagaraSystem>& System, FName SocketName = NAME_None, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, bool bAutoDestroy = true, bool bAutoActivate = true);
};
