// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MissionGameMode.generated.h"

class UWorld;

/**
 * Shared mission GameMode. Mission 1's exit and Mission 2's enemy counter both call
 * CompleteMission(); there is no objective-type enum or virtual objective hook by design.
 */
UCLASS()
class HACKGAMEAI_API AMissionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMissionGameMode();

	UPROPERTY(EditDefaultsOnly, Category = "Mission")
	TSoftObjectPtr<UWorld> NextMissionMap;

	UPROPERTY(EditDefaultsOnly, Category = "Mission")
	bool bIsFinalMission = false;

	UPROPERTY(EditDefaultsOnly, Category = "Mission")
	TSoftObjectPtr<UWorld> MainMenuMap;

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CompleteMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void FailMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void OpenNextMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void RetryMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void ReturnToMenu();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void OnMissionCompleted(bool bIsFinal);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void OnMissionFailed();

private:
	bool bMissionEnded = false;
};
