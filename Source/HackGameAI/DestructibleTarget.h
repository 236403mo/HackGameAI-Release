// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleTarget.generated.h"

class UStaticMeshComponent;
class UHealthComponent;
class UMaterialInstanceDynamic;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetDestroyed);

/**
 * Inert stationary target introduced in Mission 1. Does not move, does not shoot,
 * and has no perception or AI of any kind. Takes damage from player projectiles via
 * the shared UHealthComponent and broadcasts OnTargetDestroyed when its health reaches zero.
 *
 * Also drives simple local hit feedback (emissive flash + scale pulse) on every hit, with a
 * stronger version on the killing hit. This is intentionally local to this actor, not a
 * reusable hit-reaction framework.
 */
UCLASS()
class HACKGAMEAI_API ADestructibleTarget : public AActor
{
	GENERATED_BODY()

public:
	ADestructibleTarget();

	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnTargetDestroyed OnTargetDestroyed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	// --- Hit feedback tuning (local to this actor; not a general framework) ---

	// Emissive Intensity parameter value used for the flash on a normal (non-killing) hit.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float HitFlashIntensity = 9.0f;

	// How long the normal hit flash/pulse takes to decay back to rest, in seconds.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float HitFlashDuration = 0.15f;

	// Fractional uniform scale added at the peak of a normal hit's pulse (0.1 = +10%).
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float HitPulseScale = 0.12f;

	// Emissive Intensity parameter value used for the stronger flash on the killing hit.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float DeathFlashIntensity = 16.0f;

	// How long the killing-hit flash/pulse takes to decay, in seconds. Must fit within DeathDelay.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float DeathFlashDuration = 0.3f;

	// Fractional uniform scale added at the peak of the killing hit's pulse.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float DeathPulseScale = 0.3f;

	// Delay between the killing hit and the actor's actual Destroy(), so the stronger
	// death feedback is visible. Collision is removed and OnTargetDestroyed fires immediately
	// on the killing hit, well before this delay elapses.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float DeathDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> DamageSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> DestroyedSound;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDeath();

private:
	void PlayHitFeedback(bool bKillingHit);
	void FinishDeath();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	float RestingIntensity = 2.2f;
	FLinearColor RestingBaseColor = FLinearColor(1.0f, 0.22f, 0.05f, 1.0f);
	FVector RestingScale = FVector::OneVector;

	bool bFeedbackActive = false;
	float FeedbackElapsed = 0.0f;
	float ActiveFlashIntensity = 0.0f;
	float ActiveFlashDuration = 0.15f;
	float ActivePulseScale = 0.0f;

	FTimerHandle DeathTimerHandle;
};
