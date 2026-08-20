// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HackPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UHealthComponent;
class UInputMappingContext;
class UInputAction;
class AHackProjectile;
class USoundBase;
struct FInputActionValue;

/**
 * Player pawn. Movement is world-space via CharacterMovementComponent; facing follows the
 * controller's yaw, which the PlayerController drives from the mouse aim ray.
 */
UCLASS()
class HACKGAMEAI_API AHackPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHackPlayerCharacter();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AHackProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireCooldown = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector MuzzleOffset = FVector(120.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> FireSound;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void HandleMove(const FInputActionValue& Value);
	void HandleFire(const FInputActionValue& Value);

	UFUNCTION()
	void HandleDeath();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

private:
	float LastFireTime = -1000.0f;
};
