// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HackGameAITypes.generated.h"

/**
 * Team affiliation used for damage filtering between projectiles and health components.
 */
UENUM(BlueprintType)
enum class EHackTeam : uint8
{
	Player,
	Enemy
};
