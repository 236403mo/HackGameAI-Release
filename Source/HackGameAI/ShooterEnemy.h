// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterEnemy.generated.h"

class UStaticMeshComponent;
class UHealthComponent;
class AHackProjectile;
class UMaterialInstanceDynamic;
class USoundBase;

/**
 * Stationary hostile enemy introduced in Mission 2. Never moves and is deliberately dumb:
 * no perception, no line-of-sight trace, no range check (GAME_DESIGN.md section 6). It fires
 * at the player on a fixed cadence regardless of distance or obstruction. Takes damage via
 * the shared UHealthComponent and is destroyed when health reaches zero.
 */
UCLASS()
class HACKGAMEAI_API AShooterEnemy : public AActor
{
	GENERATED_BODY()

public:
	AShooterEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Purely visual, forward-facing protrusion attached to Mesh so it turns with the body.
	// A plain symmetric cube gives no visual read of facing at all - this is what makes the
	// enemy's yaw rotation (see Tick) actually perceivable to the player. Mesh/material are
	// assigned in the Blueprint, same convention as Mesh itself.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Barrel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	// Seconds between shots. Design range 1.3 - 1.6s.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireInterval = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AHackProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector MuzzleOffset = FVector(120.0f, 0.0f, 0.0f);

	// Max distance to the player at which this enemy will attempt to fire, in uu. Roughly one
	// camera screen's worth of ground at this project's fixed camera framing (~1400 arm length,
	// -60 pitch) - a simple world-space threshold, not an actual viewport projection. Facing/aim
	// tracking (Tick) is unaffected by this - only firing is gated.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FiringRange = 1800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> DamageSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> DestroyedSound;

	// Delay between the killing hit and Destroy(), mirroring ADestructibleTarget so the
	// death pattern stays consistent across enemy actors.
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathDelay = 0.2f;

	// --- Hit feedback tuning (local to this actor; mirrors ADestructibleTarget's pattern).
	// Drives three separate dynamic material instances - Mesh's BodyBrightness, Barrel's
	// EmissiveStrength, and the Blueprint-added "TopPanel" plane's GlowStrength - so the whole
	// enemy brightens together on a hit, not just one part. These materials have changed
	// names/baselines more than once during development, so the flash peak is expressed as a
	// multiplier of whatever the resting value actually is, not a hardcoded absolute - that's
	// what silently broke last time. ---

	// Multiplier applied to each material's resting brightness value for the flash peak (e.g.
	// 3.0 = 3x brighter than normal for an instant, then decays back to resting over
	// HitFlashDuration). Shared across Mesh/Barrel/TopPanel since it's expressed relative to
	// each one's own resting value, not an absolute. These materials' brightness values sit well
	// into the tonemapper's highlight compression, so this needs to be large to read as visibly
	// different from resting, not just numerically different.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float HitFlashIntensityMultiplier = 36.0f;

	// How long the hit flash takes to decay back to rest, in seconds.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float HitFlashDuration = 0.2f;

	// Fractional uniform scale added at the peak of the pulse (0.12 = +12%), matching the Wall
	// enemy's (ADestructibleTarget's) existing hit-feedback pulse so both read consistently.
	UPROPERTY(EditDefaultsOnly, Category = "Hit Feedback")
	float HitPulseScale = 0.12f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

private:
	void Fire();
	void FinishDeath();
	void PlayHitFlash();

	// Shared by Tick (visible facing) and Fire (projectile spawn rotation) so both always
	// agree on where the enemy is aiming. Flattened to yaw only, matching the player's own
	// aim math. Returns false (and leaves OutRotation untouched) if there is no player pawn
	// to aim at, e.g. between mission transitions.
	bool ComputeAimRotationToPlayer(FRotator& OutRotation) const;

	FTimerHandle FireTimerHandle;
	FTimerHandle DeathTimerHandle;

	// Dynamic instances created at BeginPlay to drive the hit flash on the body, barrel, and
	// top panel.
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyDynamicMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BarrelDynamicMaterial;

	// TopPanel is added per-Blueprint (BP_ShooterEnemy, BP_MovingShooterEnemy,
	// BP_CaterpillarSegment), not a native component like Mesh/Barrel, so it's looked up by name
	// at BeginPlay rather than declared as a UPROPERTY here.
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> TopPanelDynamicMaterial;

	float RestingBodyBrightness = 1.5f;
	float RestingBarrelIntensity = 150.0f;
	float RestingTopPanelGlow = 1000.0f;
	FVector RestingScale = FVector::OneVector;

	bool bHitFlashActive = false;
	float HitFlashElapsed = 0.0f;
};
