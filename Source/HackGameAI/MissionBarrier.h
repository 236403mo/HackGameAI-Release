// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionBarrier.generated.h"

class UStaticMeshComponent;

/**
 * Physical gate that blocks the player and projectiles until every actor listed in
 * RequiredEnemies has been destroyed. Purely physical: it only blocks/unblocks the path.
 * It does not call CompleteMission() — mission completion stays with AMissionExit.
 */
UCLASS()
class HACKGAMEAI_API AMissionBarrier : public AActor
{
	GENERATED_BODY()

public:
	AMissionBarrier();

	// Enemies that must be destroyed before this barrier unlocks. Assigned per placed
	// instance in the level, mirroring AMissionExit::RequiredTarget.
	UPROPERTY(EditInstanceOnly, Category = "Mission")
	TArray<TObjectPtr<AActor>> RequiredEnemies;

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsLocked() const { return bIsLocked; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleRequiredEnemyDeath();

	// Fired when the barrier transitions from locked to unlocked, so Blueprint can add a
	// visual response (e.g. swap material, play an effect) later.
	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void OnBarrierUnlocked();

private:
	void Unlock();

	bool bIsLocked = true;
	int32 RemainingEnemyCount = 0;
};
