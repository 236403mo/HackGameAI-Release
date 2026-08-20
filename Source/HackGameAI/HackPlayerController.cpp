// Copyright Epic Games, Inc. All Rights Reserved.

#include "HackPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "HackPlayerCharacter.h"
#include "GameFramework/Pawn.h"

AHackPlayerController::AHackPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShouldPerformFullTickWhenPaused = true;
}

void AHackPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	CurrentMouseCursor = EMouseCursor::Crosshairs;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (const AHackPlayerCharacter* HackCharacter = Cast<AHackPlayerCharacter>(GetPawn()))
			{
				if (HackCharacter->InputMappingContext)
				{
					Subsystem->AddMappingContext(HackCharacter->InputMappingContext, 0);
				}
			}
		}
	}
}

void AHackPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// This controller sets bShouldPerformFullTickWhenPaused so pause input keeps working,
	// which means Tick() still runs while the pause menu or mission-complete overlay has
	// the game paused. Aiming is gameplay state, so it must not keep running there:
	// without this guard UpdateMouseAim() keeps calling SetControlRotation() behind the
	// overlay, rotating the pawn and view while the game is supposed to be frozen.
	if (IsPaused())
	{
		return;
	}

	UpdateMouseAim();
}

void AHackPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AHackPlayerController::HandlePause);
		}
	}
}

void AHackPlayerController::HandlePause(const FInputActionValue& Value)
{
	const bool bNewPaused = !IsPaused();
	SetPause(bNewPaused);
	OnPauseToggled(bNewPaused);
}

void AHackPlayerController::UpdateMouseAim()
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FVector WorldLocation;
	FVector WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return;
	}

	// Guard against a ray nearly parallel to the aim plane, which would blow up the intersection.
	if (FMath::IsNearlyZero(WorldDirection.Z, 0.0001f))
	{
		return;
	}

	const float PlaneZ = ControlledPawn->GetActorLocation().Z;
	const float T = (PlaneZ - WorldLocation.Z) / WorldDirection.Z;
	if (T < 0.0f)
	{
		return;
	}

	const FVector HitPoint = WorldLocation + WorldDirection * T;
	const FVector ToHit = HitPoint - ControlledPawn->GetActorLocation();
	if (ToHit.IsNearlyZero())
	{
		return;
	}

	const FRotator AimRotation(0.0f, ToHit.Rotation().Yaw, 0.0f);
	SetControlRotation(AimRotation);
}
