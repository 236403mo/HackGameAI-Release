// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionExit.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ADestructibleTarget;

/**
 * Overlap trigger that completes the mission when the player pawn reaches it.
 * If RequiredTarget is set, the exit stays inactive (overlap does nothing, and the
 * once-only guard is not consumed) until that target is destroyed. Fires at most once.
 */
UCLASS()
class HACKGAMEAI_API AMissionExit : public AActor
{
	GENERATED_BODY()

public:
	AMissionExit();

	// Optional target that must be destroyed before this exit activates. If left unset,
	// the exit behaves as always-active, matching earlier mission shells.
	UPROPERTY(EditInstanceOnly, Category = "Mission")
	TObjectPtr<ADestructibleTarget> RequiredTarget;

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsActive() const { return bIsActive; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleRequiredTargetDestroyed();

	// Fired when the exit transitions from inactive to active, so Blueprint can swap the pad look.
	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void OnExitActivated();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PadMesh;

private:
	bool bHasTriggered = false;
	bool bIsActive = false;
};
