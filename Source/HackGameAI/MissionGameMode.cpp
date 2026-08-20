// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AMissionGameMode::AMissionGameMode()
{
}

void AMissionGameMode::CompleteMission()
{
	if (bMissionEnded)
	{
		return;
	}
	bMissionEnded = true;

	UGameplayStatics::SetGamePaused(this, true);
	OnMissionCompleted(bIsFinalMission);
}

void AMissionGameMode::FailMission()
{
	if (bMissionEnded)
	{
		return;
	}
	bMissionEnded = true;

	UGameplayStatics::SetGamePaused(this, true);
	OnMissionFailed();
}

void AMissionGameMode::OpenNextMission()
{
	UGameplayStatics::SetGamePaused(this, false);

	if (!NextMissionMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, NextMissionMap);
	}
}

void AMissionGameMode::RetryMission()
{
	UGameplayStatics::SetGamePaused(this, false);

	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AMissionGameMode::ReturnToMenu()
{
	UGameplayStatics::SetGamePaused(this, false);

	if (!MainMenuMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuMap);
	}
}
