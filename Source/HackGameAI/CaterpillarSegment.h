// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterEnemy.h"
#include "CaterpillarSegment.generated.h"

class ACaterpillarPath;

/**
 * One boss body segment for the Mission 3 "Caterpillar" boss. Subclasses AShooterEnemy unchanged
 * for aiming/firing/health/death; adds deterministic path-following movement on top. All 8
 * placed instances share the same ACaterpillarPath and MoveSpeed, differing only by a fixed
 * PathOffset - this is what makes them move as a connected train with zero drift risk (identical
 * per-frame increment for all 8, offset is constant forever). Destruction of an individual
 * segment requires no special handling: each segment is an independent actor with its own
 * inherited HealthComponent/OnDeath/timer, exactly like every other AShooterEnemy subclass.
 */
UCLASS()
class HACKGAMEAI_API ACaterpillarSegment : public AShooterEnemy
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Category = "Movement")
	TObjectPtr<ACaterpillarPath> Path;

	// This segment's fixed distance offset along the path relative to the others (e.g. segment
	// 1 = 0, segment 2 = -150, segment 3 = -300, ... for evenly trailing "train cars").
	UPROPERTY(EditInstanceOnly, Category = "Movement")
	float PathOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MoveSpeed = 150.0f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	float DistanceTraveled = 0.0f;

	// Guards the missing-Path warning so it logs once instead of every frame.
	bool bWarnedMissingPath = false;
};
