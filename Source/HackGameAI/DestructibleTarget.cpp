// Copyright Epic Games, Inc. All Rights Reserved.

#include "DestructibleTarget.h"
#include "Components/StaticMeshComponent.h"
#include "HealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	static const FName IntensityParamName(TEXT("Intensity"));
	static const FName BaseColorParamName(TEXT("BaseColor"));
}

ADestructibleTarget::ADestructibleTarget()
{
	// Ticking is registered but starts disabled: this actor has no permanent tick logic.
	// Tick is switched on only while a hit-feedback flash/pulse is playing, and switched
	// back off the moment it finishes (see PlayHitFeedback / Tick).
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetMobility(EComponentMobility::Static);
	RootComponent = Mesh;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->Team = EHackTeam::Enemy;
	HealthComponent->MaxHealth = 3.0f;
}

void ADestructibleTarget::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &ADestructibleTarget::HandleHealthChanged);
	HealthComponent->OnDeath.AddDynamic(this, &ADestructibleTarget::HandleDeath);

	RestingScale = GetActorScale3D();

	if (Mesh)
	{
		if (UMaterialInterface* SourceMaterial = Mesh->GetMaterial(0))
		{
			// Cache the resting look before creating the dynamic instance, in case the
			// source values differ from the class defaults above.
			float ParamIntensity = RestingIntensity;
			if (SourceMaterial->GetScalarParameterValue(FMaterialParameterInfo(IntensityParamName), ParamIntensity))
			{
				RestingIntensity = ParamIntensity;
			}

			FLinearColor ParamColor = RestingBaseColor;
			if (SourceMaterial->GetVectorParameterValue(FMaterialParameterInfo(BaseColorParamName), ParamColor))
			{
				RestingBaseColor = ParamColor;
			}

			DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
		}
	}
}

void ADestructibleTarget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	// The killing hit also triggers OnHealthChanged (broadcast just before OnDeath), but it
	// should only play the stronger death feedback, not both. Skip the normal flash here
	// when this hit has brought the target to zero health.
	if (!HealthComponent || !HealthComponent->IsAlive())
	{
		return;
	}

	PlayHitFeedback(false);

	if (DamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DamageSound, GetActorLocation());
	}
}

void ADestructibleTarget::HandleDeath()
{
	if (DestroyedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroyedSound, GetActorLocation());
	}

	// 1) Open the corridor immediately: remove collision the instant the target dies.
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 2) Let the exit activate immediately, without waiting for the death feedback or Destroy().
	OnTargetDestroyed.Broadcast();

	// 3) Keep the mesh visible and play the stronger death feedback.
	PlayHitFeedback(true);

	// 4) Destroy the actor after a short, tunable delay so the feedback is visible.
	GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &ADestructibleTarget::FinishDeath, DeathDelay, false);
}

void ADestructibleTarget::FinishDeath()
{
	Destroy();
}

void ADestructibleTarget::PlayHitFeedback(bool bKillingHit)
{
	ActiveFlashIntensity = bKillingHit ? DeathFlashIntensity : HitFlashIntensity;
	ActiveFlashDuration = FMath::Max(bKillingHit ? DeathFlashDuration : HitFlashDuration, 0.01f);
	ActivePulseScale = bKillingHit ? DeathPulseScale : HitPulseScale;

	FeedbackElapsed = 0.0f;
	bFeedbackActive = true;
	SetActorTickEnabled(true);

	// Apply the peak flash immediately so the reaction reads as instant.
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(IntensityParamName, ActiveFlashIntensity);
		DynamicMaterial->SetVectorParameterValue(BaseColorParamName, FMath::Lerp(RestingBaseColor, FLinearColor::White, 0.6f));
	}
}

void ADestructibleTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bFeedbackActive)
	{
		SetActorTickEnabled(false);
		return;
	}

	FeedbackElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(FeedbackElapsed / ActiveFlashDuration, 0.0f, 1.0f);

	if (DynamicMaterial)
	{
		const float Intensity = FMath::Lerp(ActiveFlashIntensity, RestingIntensity, Alpha);
		DynamicMaterial->SetScalarParameterValue(IntensityParamName, Intensity);

		const FLinearColor FlashColor = FMath::Lerp(RestingBaseColor, FLinearColor::White, 0.6f);
		DynamicMaterial->SetVectorParameterValue(BaseColorParamName, FMath::Lerp(FlashColor, RestingBaseColor, Alpha));
	}

	// Squash/pulse: scale rises then eases back to rest across the same window.
	const float PulseFactor = FMath::Sin(Alpha * PI) * ActivePulseScale;
	SetActorScale3D(RestingScale * (1.0f + PulseFactor));

	if (Alpha >= 1.0f)
	{
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(IntensityParamName, RestingIntensity);
			DynamicMaterial->SetVectorParameterValue(BaseColorParamName, RestingBaseColor);
		}
		SetActorScale3D(RestingScale);

		bFeedbackActive = false;
		SetActorTickEnabled(false);
	}
}
