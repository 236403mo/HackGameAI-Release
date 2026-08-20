// Copyright Epic Games, Inc. All Rights Reserved.

#include "CaterpillarSegment.h"
#include "CaterpillarPath.h"

void ACaterpillarSegment::BeginPlay()
{
	// Preserves the inherited fire-timer setup and HealthComponent OnDeath binding - do not
	// duplicate or bypass it.
	Super::BeginPlay();

	DistanceTraveled = PathOffset;
}

void ACaterpillarSegment::Tick(float DeltaSeconds)
{
	// Runs the inherited rotation-to-face-player logic first. This is the only place player
	// state is read; the movement below is fully decoupled from it and never influenced by the
	// player.
	Super::Tick(DeltaSeconds);

	DistanceTraveled += MoveSpeed * DeltaSeconds;

	if (Path)
	{
		SetActorLocation(Path->GetLocationAtDistance(DistanceTraveled), /*bSweep=*/false);
	}
	else if (!bWarnedMissingPath)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACaterpillarSegment '%s' has no Path assigned; it will aim and fire but not move."), *GetName());
		bWarnedMissingPath = true;
	}
}
