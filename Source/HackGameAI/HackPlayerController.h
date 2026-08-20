// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HackPlayerController.generated.h"

class UInputAction;
struct FInputActionValue;

/**
 * Drives mouse aiming by intersecting the mouse ray with a horizontal plane at the pawn's
 * height, and owns the pause input. Never traces world geometry for aiming.
 */
UCLASS()
class HACKGAMEAI_API AHackPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHackPlayerController();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause")
	void OnPauseToggled(bool bIsPaused);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	void HandlePause(const FInputActionValue& Value);
	void UpdateMouseAim();
};
