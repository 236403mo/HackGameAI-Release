// Copyright Epic Games, Inc. All Rights Reserved.

#include "HackPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "HealthComponent.h"
#include "HackProjectile.h"
#include "HackGameAITypes.h"
#include "MissionGameMode.h"
#include "Kismet/GameplayStatics.h"

AHackPlayerCharacter::AHackPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(GetCapsuleComponent());
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 1400.0f;
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.0f;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 5.0f;
	HealthComponent->Team = EHackTeam::Player;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->MaxWalkSpeed = 600.0f;
		MoveComp->MaxAcceleration = 4000.0f;
		MoveComp->BrakingDecelerationWalking = 4000.0f;
		MoveComp->GroundFriction = 8.0f;
	}
}

void AHackPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddDynamic(this, &AHackPlayerCharacter::HandleDeath);
}

void AHackPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHackPlayerCharacter::HandleMove);
		}
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AHackPlayerCharacter::HandleFire);
		}
	}
}

void AHackPlayerCharacter::HandleMove(const FInputActionValue& Value)
{
	// Gameplay is frozen while the pause or mission-complete overlay is up.
	if (UGameplayStatics::IsGamePaused(this))
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	// World-space movement: not relative to actor or camera facing, since the pawn yaws to face the mouse.
	AddMovementInput(FVector::ForwardVector, MovementVector.X);
	AddMovementInput(FVector::RightVector, MovementVector.Y);
}

void AHackPlayerCharacter::HandleFire(const FInputActionValue& Value)
{
	// Gameplay is frozen while the pause or mission-complete overlay is up.
	if (UGameplayStatics::IsGamePaused(this))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !ProjectileClass)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastFireTime < FireCooldown)
	{
		return;
	}
	LastFireTime = Now;

	const FRotator AimRotation = GetControlRotation();
	const FVector MuzzleLocation = GetActorLocation() + AimRotation.RotateVector(MuzzleOffset);

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AHackProjectile* Projectile = World->SpawnActor<AHackProjectile>(ProjectileClass, MuzzleLocation, AimRotation, SpawnParams))
	{
		Projectile->Team = EHackTeam::Player;
	}
}

void AHackPlayerCharacter::HandleDeath()
{
	if (AMissionGameMode* GameMode = Cast<AMissionGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->FailMission();
	}
}
