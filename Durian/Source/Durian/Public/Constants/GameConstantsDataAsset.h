#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameConstantsDataAsset.generated.h"

UCLASS(BlueprintType)
class DURIAN_API UGameConstantsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	static const UGameConstantsDataAsset* Get();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magnesis")
	float MagnesisTargetRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magnesis")
	float MagnesisDefaultDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magnesis")
	float MagnesisMinDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magnesis")
	float MagnesisMaxDistance = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magnesis")
	float MagnesisFollowSpeed = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis")
	float StasisTargetRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis")
	float StasisDuration = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis")
	float StasisCooldown = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis")
	float StasisImpulsePerHit = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis")
	float StasisMaxImpulse = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis|Attack")
	float StasisMeleeRange = 220.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stasis|Attack")
	float StasisMeleeRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cryonis|Animation")
	float IcePillarSpawnAnimationDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cryonis|Animation")
	float IcePillarPreviewAnimationDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cryonis")
	int32 MaxIcePillarCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cryonis")
	float IcePillarSpawnCooldown = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cryonis", meta = (ClampMin = "1.0", UIMin = "1.0", Units = "cm"))
	float IcePillarHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cryonis")
	float CryonisTargetRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombRadius = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombHoldHeight = 155.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombPickupRange = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombThrowImpulse = 950.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombExplosionRadius = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombImpulse = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombPlayerKnockback = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombDamage = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Remote Bomb")
	float RemoteBombCooldown = 6.0f;
};
