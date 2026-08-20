// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CaterpillarPath.generated.h"

class USceneComponent;

/**
 * Lightweight, mesh-less data actor describing a closed rectangular loop in world X/Y at a
 * constant Z. Placed once per boss arena; every ACaterpillarSegment references the same
 * instance. Pure geometry data queried by other actors - deliberately no Tick, no movement of
 * its own, no gameplay logic. This is the simplest deterministic closed-path solution: four
 * predefined path segments traversed in a fixed loop (GAME_DESIGN.md section 6 - no NavMesh,
 * no pathfinding, no path-choosing logic).
 */
UCLASS()
class HACKGAMEAI_API ACaterpillarPath : public AActor
{
	GENERATED_BODY()

public:
	ACaterpillarPath();

	// Axis-aligned rectangle in world X/Y.
	UPROPERTY(EditInstanceOnly, Category = "Path")
	FVector2D MinCorner = FVector2D(-1000.0f, -1000.0f);

	UPROPERTY(EditInstanceOnly, Category = "Path")
	FVector2D MaxCorner = FVector2D(1000.0f, 1000.0f);

	// Constant world Z for everything following this path (ground height).
	UPROPERTY(EditInstanceOnly, Category = "Path")
	float PathZ = 92.0f;

	UFUNCTION(BlueprintPure, Category = "Path")
	float GetPerimeter() const;

	// Walks a fixed 4-segment loop in order: top edge -> right edge -> bottom edge -> left edge
	// -> repeat. Distance is wrapped into [0, GetPerimeter()) using a proper positive modulo
	// (negative input wraps correctly rather than producing a negative or invalid result).
	UFUNCTION(BlueprintPure, Category = "Path")
	FVector GetLocationAtDistance(float Distance) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;
};
