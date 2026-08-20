// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovingShooterEnemy.h"

void AMovingShooterEnemy::BeginPlay()
{
	// Preserves the inherited fire-timer setup and HealthComponent OnDeath binding - do not
	// duplicate or bypass it.
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	PathAlpha = bStartTowardOffset ? 0.0f : 1.0f;
	DirectionSign = bStartTowardOffset ? 1.0f : -1.0f;
}

void AMovingShooterEnemy::Tick(float DeltaSeconds)
{
	// Runs the inherited rotation-to-face-player logic first. This is the only place player
	// state is read; the movement below is fully decoupled from it and never influenced by the
	// player.
	Super::Tick(DeltaSeconds);

	const float SegmentLength = FMath::Max(MoveOffset.Size(), 1.0f);
	PathAlpha += DirectionSign * (MoveSpeed * DeltaSeconds / SegmentLength);

	if (PathAlpha >= 1.0f)
	{
		PathAlpha = 1.0f;
		DirectionSign = -1.0f;
	}
	else if (PathAlpha <= 0.0f)
	{
		PathAlpha = 0.0f;
		DirectionSign = 1.0f;
	}

	// bSweep = false: this is deterministic scripted motion, not physics-driven, and must never
	// be deflected or blocked by collision response.
	SetActorLocation(StartLocation + MoveOffset * PathAlpha, /*bSweep=*/false);
}
