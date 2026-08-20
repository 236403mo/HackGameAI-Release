// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterEnemy.h"
#include "MovingShooterEnemy.generated.h"

/**
 * Corridor shooter introduced in Mission 3. Reuses AShooterEnemy's aiming/firing/health/death
 * behavior unchanged and adds a single deterministic movement pattern on top: horizontal
 * ping-pong along a fixed line segment. Movement never reads player state - only the inherited
 * Tick's aiming does that. One class, many instances differentiated purely by per-instance data
 * (MoveOffset, bStartTowardOffset), per GAME_DESIGN.md section 6 ("simple direct steering",
 * no NavMesh/AIController/Behavior Trees - here even simpler, since it is not steering at all,
 * just predefined closed-form motion).
 */
UCLASS()
class HACKGAMEAI_API AMovingShooterEnemy : public AShooterEnemy
{
	GENERATED_BODY()

public:
	// Far endpoint of the ping-pong segment is StartLocation + MoveOffset, where StartLocation
	// is wherever the actor is placed in the level. Lets a level designer place the actor at one
	// end and set a relative offset rather than computing two absolute world positions.
	UPROPERTY(EditInstanceOnly, Category = "Movement")
	FVector MoveOffset = FVector(0.0f, 300.0f, 0.0f);

	// Constant speed in uu/s along the segment.
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MoveSpeed = 200.0f;

	// Controls initial direction, so placed instances can alternate for visual variety.
	UPROPERTY(EditInstanceOnly, Category = "Movement")
	bool bStartTowardOffset = true;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	// Cached at BeginPlay = initial actor location.
	FVector StartLocation = FVector::ZeroVector;

	// 0 = at StartLocation, 1 = at StartLocation + MoveOffset.
	float PathAlpha = 0.0f;

	// +1.0 or -1.0.
	float DirectionSign = 1.0f;
};
