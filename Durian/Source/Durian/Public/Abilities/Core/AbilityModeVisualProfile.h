#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "Engine/DataAsset.h"
#include "AbilityModeVisualProfile.generated.h"

/** 모든 능력 모드가 공유하는 시각 강도 파라미터다. */
USTRUCT(BlueprintType)
struct FAbilityModeVisualCommonProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scan", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WorldBlend = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scan|Candidate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CandidateBlend = 0.34f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scan|Targeted", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TargetBlend = 0.48f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TargetEdgeOpacity = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay", meta = (ClampMin = "0.0"))
	float ScanNormalTopOverlayZOffset = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (ClampMin = "0.0"))
	float ScanTargetTopOverlayZOffset = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (ClampMin = "1.0"))
	float ScanTargetTopOverlayScale = 1.012f;

};

/** 능력 모드별 색상 시각 파라미터다. */
USTRUCT(BlueprintType)
struct FAbilityModeVisualProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay", meta = (DisplayName = "Scan Line Color"))
	FLinearColor ScanColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (DisplayName = "Candidate Main Color"))
	FLinearColor CandidateGradeColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (DisplayName = "Candidate Edge Color"))
	FLinearColor CandidateGlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (DisplayName = "Target Main Color"))
	FLinearColor TargetScanColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Surface", meta = (DisplayName = "Target Edge Color"))
	FLinearColor TargetEdgeColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scan", meta = (DisplayName = "World Grade Color"))
	FLinearColor WorldGradeColor;

};

/** 에디터에서 능력별 모든 색상과 강조 강도를 조절하는 단일 에셋이다. */
UCLASS(BlueprintType)
class DURIAN_API UAbilityModeVisualProfilesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common")
	FAbilityModeVisualCommonProfile Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Mode Visuals")
	FAbilityModeVisualProfile Magnesis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Mode Visuals")
	FAbilityModeVisualProfile Cryonis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Mode Visuals")
	FAbilityModeVisualProfile Stasis;

};

/** 모드별 시각 언어의 단일 조회점이다. */
class DURIAN_API FAbilityModeVisualProfiles final
{
public:
	static const FAbilityModeVisualCommonProfile& GetCommon();
	static const FAbilityModeVisualProfile& Get(EAbilityVisualMode Mode);
};
