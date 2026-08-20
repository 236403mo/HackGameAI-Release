// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterEnemy.h"
#include "Components/StaticMeshComponent.h"
#include "HealthComponent.h"
#include "HackProjectile.h"
#include "HackGameAITypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

namespace
{
	// These materials' assigned parameter names have changed more than once during development;
	// these are the names on whatever materials are currently assigned (M_Shooter for the body,
	// M_Shooter_Barrel for the barrel, as of this writing). If either material changes again,
	// update the corresponding name to match.
	static const FName BodyBrightnessParamName(TEXT("BodyBrightness"));
	static const FName EmissiveStrengthParamName(TEXT("EmissiveStrength"));
	static const FName GlowStrengthParamName(TEXT("GlowStrength"));
	static const FName TopPanelComponentName(TEXT("TopPanel"));
}

AShooterEnemy::AShooterEnemy()
{
	// Firing still runs on a repeating timer, not Tick - but the enemy must visibly track the
	// player's position between shots (per the design's "rotates to face player" behavior), so
	// Tick is enabled purely to update facing every frame.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	// Must be Movable, not Static: the enemy's position never changes, but Tick rotates it to
	// face the player, and Static components silently ignore runtime rotation changes.
	Mesh->SetMobility(EComponentMobility::Movable);
	RootComponent = Mesh;

	Barrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
	Barrel->SetupAttachment(Mesh);
	Barrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Barrel->SetMobility(EComponentMobility::Movable);
	Barrel->SetRelativeLocation(FVector(95.0f, 0.0f, 0.0f));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->Team = EHackTeam::Enemy;
	HealthComponent->MaxHealth = 3.0f;
}

void AShooterEnemy::BeginPlay()
{
	Super::BeginPlay();

	RestingScale = GetActorScale3D();

	HealthComponent->OnDeath.AddDynamic(this, &AShooterEnemy::HandleDeath);
	HealthComponent->OnHealthChanged.AddDynamic(this, &AShooterEnemy::HandleHealthChanged);

	// Always active: fires on a fixed cadence with no perception, line-of-sight, or range
	// check by design. Deliberately no predictive pre-fire telegraph.
	if (FireInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AShooterEnemy::Fire, FireInterval, true);
	}

	// Hit flash brightens both the body and the barrel together.
	if (Mesh)
	{
		if (UMaterialInterface* SourceMaterial = Mesh->GetMaterial(0))
		{
			float ParamBrightness = RestingBodyBrightness;
			if (SourceMaterial->GetScalarParameterValue(FMaterialParameterInfo(BodyBrightnessParamName), ParamBrightness))
			{
				RestingBodyBrightness = ParamBrightness;
			}

			BodyDynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
		}
	}

	if (Barrel)
	{
		if (UMaterialInterface* SourceMaterial = Barrel->GetMaterial(0))
		{
			float ParamIntensity = RestingBarrelIntensity;
			if (SourceMaterial->GetScalarParameterValue(FMaterialParameterInfo(EmissiveStrengthParamName), ParamIntensity))
			{
				RestingBarrelIntensity = ParamIntensity;
			}

			BarrelDynamicMaterial = Barrel->CreateAndSetMaterialInstanceDynamic(0);
		}
	}

	// TopPanel is added per-Blueprint, not a native component, so find it by name.
	for (UActorComponent* Component : GetComponents())
	{
		UStaticMeshComponent* TopPanel = Cast<UStaticMeshComponent>(Component);
		if (TopPanel && TopPanel->GetFName() == TopPanelComponentName)
		{
			if (UMaterialInterface* SourceMaterial = TopPanel->GetMaterial(0))
			{
				float ParamGlow = RestingTopPanelGlow;
				if (SourceMaterial->GetScalarParameterValue(FMaterialParameterInfo(GlowStrengthParamName), ParamGlow))
				{
					RestingTopPanelGlow = ParamGlow;
				}

				TopPanelDynamicMaterial = TopPanel->CreateAndSetMaterialInstanceDynamic(0);
			}
			break;
		}
	}
}

void AShooterEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FRotator AimRotation;
	if (ComputeAimRotationToPlayer(AimRotation))
	{
		SetActorRotation(AimRotation);
	}

	// Hit flash decay: only runs while a flash is active, layered on top of the tick this
	// actor already runs every frame for aim rotation (no SetActorTickEnabled toggling needed).
	if (bHitFlashActive)
	{
		HitFlashElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(HitFlashElapsed / FMath::Max(HitFlashDuration, 0.01f), 0.0f, 1.0f);

		if (BodyDynamicMaterial)
		{
			const float PeakBrightness = RestingBodyBrightness * HitFlashIntensityMultiplier;
			const float Brightness = FMath::Lerp(PeakBrightness, RestingBodyBrightness, Alpha);
			BodyDynamicMaterial->SetScalarParameterValue(BodyBrightnessParamName, Brightness);
		}

		if (BarrelDynamicMaterial)
		{
			const float PeakIntensity = RestingBarrelIntensity * HitFlashIntensityMultiplier;
			const float Intensity = FMath::Lerp(PeakIntensity, RestingBarrelIntensity, Alpha);
			BarrelDynamicMaterial->SetScalarParameterValue(EmissiveStrengthParamName, Intensity);
		}

		if (TopPanelDynamicMaterial)
		{
			const float PeakGlow = RestingTopPanelGlow * HitFlashIntensityMultiplier;
			const float Glow = FMath::Lerp(PeakGlow, RestingTopPanelGlow, Alpha);
			TopPanelDynamicMaterial->SetScalarParameterValue(GlowStrengthParamName, Glow);
		}

		// Squash/pulse: scale rises then eases back to rest across the same window, matching
		// the Wall enemy's (ADestructibleTarget's) existing hit-feedback pulse.
		const float PulseFactor = FMath::Sin(Alpha * PI) * HitPulseScale;
		SetActorScale3D(RestingScale * (1.0f + PulseFactor));

		if (Alpha >= 1.0f)
		{
			if (BodyDynamicMaterial)
			{
				BodyDynamicMaterial->SetScalarParameterValue(BodyBrightnessParamName, RestingBodyBrightness);
			}
			if (BarrelDynamicMaterial)
			{
				BarrelDynamicMaterial->SetScalarParameterValue(EmissiveStrengthParamName, RestingBarrelIntensity);
			}
			if (TopPanelDynamicMaterial)
			{
				TopPanelDynamicMaterial->SetScalarParameterValue(GlowStrengthParamName, RestingTopPanelGlow);
			}
			SetActorScale3D(RestingScale);
			bHitFlashActive = false;
		}
	}
}

void AShooterEnemy::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	// The killing hit also triggers OnHealthChanged, but death handling (HandleDeath /
	// FinishDeath) already covers it; skip the hit flash there so it doesn't fight the
	// collision/timer changes death makes.
	if (!HealthComponent || !HealthComponent->IsAlive())
	{
		return;
	}

	PlayHitFlash();

	if (DamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DamageSound, GetActorLocation());
	}
}

void AShooterEnemy::PlayHitFlash()
{
	HitFlashElapsed = 0.0f;
	bHitFlashActive = true;

	// Apply the peak flash immediately so the reaction reads as instant.
	if (BodyDynamicMaterial)
	{
		BodyDynamicMaterial->SetScalarParameterValue(BodyBrightnessParamName, RestingBodyBrightness * HitFlashIntensityMultiplier);
	}
	if (BarrelDynamicMaterial)
	{
		BarrelDynamicMaterial->SetScalarParameterValue(EmissiveStrengthParamName, RestingBarrelIntensity * HitFlashIntensityMultiplier);
	}
	if (TopPanelDynamicMaterial)
	{
		TopPanelDynamicMaterial->SetScalarParameterValue(GlowStrengthParamName, RestingTopPanelGlow * HitFlashIntensityMultiplier);
	}
}

bool AShooterEnemy::ComputeAimRotationToPlayer(FRotator& OutRotation) const
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		// No player pawn to aim at (e.g. between mission transitions).
		return false;
	}

	const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	if (ToPlayer.IsNearlyZero())
	{
		return false;
	}

	// Flatten to the gameplay plane, matching how the player's own aim is computed, so both
	// facing and projectiles stay horizontal regardless of small actor-height differences.
	OutRotation = FRotator(0.0f, ToPlayer.Rotation().Yaw, 0.0f);
	return true;
}

void AShooterEnemy::Fire()
{
	UWorld* World = GetWorld();
	if (!World || !ProjectileClass)
	{
		return;
	}

	FRotator AimRotation;
	if (!ComputeAimRotationToPlayer(AimRotation))
	{
		// No player pawn to aim at (e.g. between mission transitions): skip this shot rather than crash.
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation()) > FiringRange)
	{
		// Out of firing range: keep aiming (Tick still tracks the player every frame), just skip the shot,
		// its projectile, and its sound.
		return;
	}

	const FVector MuzzleLocation = GetActorLocation() + AimRotation.RotateVector(MuzzleOffset);

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AHackProjectile* Projectile = World->SpawnActor<AHackProjectile>(ProjectileClass, MuzzleLocation, AimRotation, SpawnParams))
	{
		Projectile->Team = EHackTeam::Enemy;
	}
}

void AShooterEnemy::HandleDeath()
{
	if (DestroyedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroyedSound, GetActorLocation());
	}

	// Stop firing immediately on death.
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	// Open the corridor immediately: remove collision the instant the enemy dies, mirroring
	// ADestructibleTarget's death handling.
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &AShooterEnemy::FinishDeath, DeathDelay, false);
}

void AShooterEnemy::FinishDeath()
{
	Destroy();
}
