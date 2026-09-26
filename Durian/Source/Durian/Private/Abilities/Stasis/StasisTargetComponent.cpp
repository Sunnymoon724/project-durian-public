#include "Abilities/Stasis/StasisTargetComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

bool UStasisTargetComponent::BeginStasis(UPrimitiveComponent* InPrimitive)
{
	if (bStasisActive || !IsValid(InPrimitive) || !InPrimitive->IsSimulatingPhysics() || !IsValid(GetOwner()))
	{
		return false;
	}

	FrozenPrimitive = InPrimitive;
	SavedLinearVelocity = InPrimitive->GetPhysicsLinearVelocity();
	SavedAngularVelocity = InPrimitive->GetPhysicsAngularVelocityInRadians();
	bSavedGravityEnabled = InPrimitive->IsGravityEnabled();
	bSavedCanBeDamaged = GetOwner()->CanBeDamaged();
	AccumulatedImpulse = FVector::ZeroVector;
	bStasisActive = true;

	GetOwner()->SetCanBeDamaged(false);
	InPrimitive->SetSimulatePhysics(false);
	CreateFeedback();
	SpawnPulse();
	return true;
}

void UStasisTargetComponent::AccumulateImpulse(const FVector& Impulse, const float MaxImpulse)
{
	if (!bStasisActive || Impulse.IsNearlyZero())
	{
		return;
	}

	AccumulatedImpulse = (AccumulatedImpulse + Impulse).GetClampedToMaxSize(FMath::Max(0.0f, MaxImpulse));
	if (UNiagaraSystem* HitSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Niagara/NS_StasisForceHit.NS_StasisForceHit")))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitSystem, FrozenPrimitive->Bounds.Origin);
	}
}

void UStasisTargetComponent::EndStasis(const bool bApplyImpulse)
{
	if (!bStasisActive)
	{
		return;
	}

	bStasisActive = false;
	if (bApplyImpulse)
	{
		if (UNiagaraSystem* ReleaseSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Niagara/NS_StasisRelease.NS_StasisRelease")))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ReleaseSystem, FrozenPrimitive->Bounds.Origin);
		}
	}
	ClearFeedback();
	if (AActor* Owner = GetOwner())
	{
		Owner->SetCanBeDamaged(bSavedCanBeDamaged);
	}

	if (UPrimitiveComponent* Primitive = FrozenPrimitive.Get())
	{
		Primitive->SetSimulatePhysics(true);
		Primitive->SetEnableGravity(bSavedGravityEnabled);
		Primitive->SetPhysicsLinearVelocity(SavedLinearVelocity);
		Primitive->SetPhysicsAngularVelocityInRadians(SavedAngularVelocity);
		if (bApplyImpulse && !AccumulatedImpulse.IsNearlyZero())
		{
			Primitive->AddImpulse(AccumulatedImpulse);
		}
	}

	FrozenPrimitive.Reset();
	AccumulatedImpulse = FVector::ZeroVector;
}

void UStasisTargetComponent::UpdateFeedback(const float RemainingTime, const float TotalTime, const FVector& ViewerLocation, const float MaxImpulse)
{
	if (!bStasisActive || !FrozenPrimitive.IsValid())
	{
		return;
	}

	const FBoxSphereBounds Bounds = FrozenPrimitive->Bounds;
	const FVector Top = Bounds.Origin + FVector(0.0f, 0.0f, Bounds.BoxExtent.Z + 45.0f);
	const float Strength = AccumulatedImpulse.Size();
	if (StatusText)
	{
		StatusText->SetWorldLocation(Top + FVector(0.0f, 0.0f, 45.0f));
		StatusText->SetWorldRotation((ViewerLocation - StatusText->GetComponentLocation()).Rotation());
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("%.1fs  %.0f / %.0f"), FMath::Max(0.0f, RemainingTime), Strength, MaxImpulse)));
	}

	const bool bHasImpulse = Strength > KINDA_SMALL_NUMBER;
	if (DirectionShaft && DirectionTip)
	{
		DirectionShaft->SetVisibility(bHasImpulse);
		DirectionTip->SetVisibility(bHasImpulse);
		if (bHasImpulse)
		{
			const FVector Direction = AccumulatedImpulse.GetSafeNormal();
			const float Length = FMath::Lerp(75.0f, 220.0f, FMath::Clamp(Strength / FMath::Max(MaxImpulse, 1.0f), 0.0f, 1.0f));
			const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Direction);
			DirectionShaft->SetWorldTransform(FTransform(Rotation, Top + Direction * (Length * 0.5f), FVector(0.08f, 0.08f, Length / 100.0f)));
			DirectionTip->SetWorldTransform(FTransform(Rotation, Top + Direction * Length, FVector(0.2f, 0.2f, 0.25f)));
		}
	}

	if (ExpiryWarning && RemainingTime <= FMath::Min(2.0f, TotalTime * 0.2f) && !ExpiryWarning->IsActive())
	{
		ExpiryWarning->Activate(true);
	}
}

void UStasisTargetComponent::CreateFeedback()
{
	UPrimitiveComponent* Primitive = FrozenPrimitive.Get();
	AActor* Owner = GetOwner();
	if (!Primitive || !Owner || !GetWorld())
	{
		return;
	}

	if (UClass* MarkerClass = LoadClass<AActor>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Blueprints/BP_StasisTargetVFX.BP_StasisTargetVFX_C")))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Owner;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		MarkerActor = GetWorld()->SpawnActor<AActor>(MarkerClass, Primitive->Bounds.Origin + FVector(0.0f, 0.0f, Primitive->Bounds.BoxExtent.Z + 20.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->SetActorEnableCollision(false);
			MarkerActor->AttachToComponent(Primitive, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	StatusText = NewObject<UTextRenderComponent>(Owner);
	StatusText->ComponentTags.Add(TEXT("StasisFeedback"));
	StatusText->RegisterComponent();
	StatusText->AttachToComponent(Primitive, FAttachmentTransformRules::KeepWorldTransform);
	StatusText->SetHorizontalAlignment(EHTA_Center);
	StatusText->SetWorldSize(28.0f);
	StatusText->SetTextRenderColor(FColor::Yellow);
	StatusText->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	UMaterialInterface* MarkerMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Materials/MI_StasisForceArrow.MI_StasisForceArrow"));
	if (Cylinder && Cone)
	{
		DirectionShaft = NewObject<UStaticMeshComponent>(Owner);
		DirectionShaft->ComponentTags.Add(TEXT("StasisFeedback"));
		DirectionShaft->SetStaticMesh(Cylinder);
		DirectionShaft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DirectionShaft->SetCastShadow(false);
		DirectionShaft->RegisterComponent();
		DirectionShaft->AttachToComponent(Primitive, FAttachmentTransformRules::KeepWorldTransform);
		DirectionShaft->SetVisibility(false);
		DirectionTip = NewObject<UStaticMeshComponent>(Owner);
		DirectionTip->ComponentTags.Add(TEXT("StasisFeedback"));
		DirectionTip->SetStaticMesh(Cone);
		DirectionTip->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DirectionTip->SetCastShadow(false);
		DirectionTip->RegisterComponent();
		DirectionTip->AttachToComponent(Primitive, FAttachmentTransformRules::KeepWorldTransform);
		DirectionTip->SetVisibility(false);
		if (MarkerMaterial)
		{
			DirectionShaft->SetMaterial(0, MarkerMaterial);
			DirectionTip->SetMaterial(0, MarkerMaterial);
		}
	}

	if (UNiagaraSystem* HoldSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Niagara/NS_StasisHoldAmbient.NS_StasisHoldAmbient")))
	{
		HoldAmbient = UNiagaraFunctionLibrary::SpawnSystemAttached(HoldSystem, Primitive, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
	}
	if (UNiagaraSystem* WarningSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Niagara/NS_StasisExpiryWarning.NS_StasisExpiryWarning")))
	{
		ExpiryWarning = UNiagaraFunctionLibrary::SpawnSystemAttached(WarningSystem, Primitive, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, false);
	}
}

void UStasisTargetComponent::ClearFeedback()
{
	if (MarkerActor)
	{
		MarkerActor->Destroy();
		MarkerActor = nullptr;
	}
	if (StatusText)
	{
		StatusText->DestroyComponent();
		StatusText = nullptr;
	}
	if (DirectionShaft)
	{
		DirectionShaft->DestroyComponent();
		DirectionShaft = nullptr;
	}
	if (DirectionTip)
	{
		DirectionTip->DestroyComponent();
		DirectionTip = nullptr;
	}
	if (HoldAmbient)
	{
		HoldAmbient->DeactivateImmediate();
		HoldAmbient->DestroyComponent();
		HoldAmbient = nullptr;
	}
	if (ExpiryWarning)
	{
		ExpiryWarning->DeactivateImmediate();
		ExpiryWarning->DestroyComponent();
		ExpiryWarning = nullptr;
	}
}

void UStasisTargetComponent::SpawnPulse() const
{
	if (!GetWorld() || !FrozenPrimitive.IsValid())
	{
		return;
	}
	if (UNiagaraSystem* Pulse = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Niagara/NS_StasisLockPulse_FromEffect.NS_StasisLockPulse_FromEffect")))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Pulse, FrozenPrimitive->Bounds.Origin);
	}
}

void UStasisTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndStasis(false);
	Super::EndPlay(EndPlayReason);
}
