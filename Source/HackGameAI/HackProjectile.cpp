// Copyright Epic Games, Inc. All Rights Reserved.

#include "HackProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"

AHackProjectile::AHackProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComponent;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 3.0f;
}

void AHackProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AHackProjectile::HandleHit);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AHackProjectile::HandleOverlap);

	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

void AHackProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Blocking hits cover non-Pawn actors (e.g. a static-mesh destructible target) that the
	// Pawn-overlap path in HandleOverlap never sees. Apply the same team-filtered damage here
	// before destroying the projectile.
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		if (UHealthComponent* OtherHealth = OtherActor->FindComponentByClass<UHealthComponent>())
		{
			if (OtherHealth->Team != Team)
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
			}
		}
	}

	Destroy();
}

void AHackProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	UHealthComponent* OtherHealth = OtherActor->FindComponentByClass<UHealthComponent>();
	if (!OtherHealth)
	{
		// No health component: ignore and keep flying (e.g. overlapping another projectile or a non-combatant pawn).
		return;
	}

	if (OtherHealth->Team == Team)
	{
		// Same team: ignore and keep flying, friendly fire is structurally impossible.
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
	Destroy();
}
