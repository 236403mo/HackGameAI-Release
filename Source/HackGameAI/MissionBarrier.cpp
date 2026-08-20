// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionBarrier.h"
#include "Components/StaticMeshComponent.h"
#include "HealthComponent.h"

AMissionBarrier::AMissionBarrier()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetMobility(EComponentMobility::Static);
	RootComponent = Mesh;
}

void AMissionBarrier::BeginPlay()
{
	Super::BeginPlay();

	int32 AliveCount = 0;
	for (AActor* Enemy : RequiredEnemies)
	{
		// Skip entries that are already null/invalid rather than counting them as alive,
		// mirroring how AMissionExit treats a null RequiredTarget.
		if (!IsValid(Enemy))
		{
			continue;
		}

		UHealthComponent* EnemyHealth = Enemy->FindComponentByClass<UHealthComponent>();
		if (!EnemyHealth)
		{
			continue;
		}

		EnemyHealth->OnDeath.AddDynamic(this, &AMissionBarrier::HandleRequiredEnemyDeath);
		++AliveCount;
	}

	RemainingEnemyCount = AliveCount;

	if (RemainingEnemyCount <= 0)
	{
		// No (valid) required enemies: treat the barrier as already unlocked.
		Unlock();
	}
}

void AMissionBarrier::HandleRequiredEnemyDeath()
{
	if (!bIsLocked)
	{
		return;
	}

	--RemainingEnemyCount;
	if (RemainingEnemyCount <= 0)
	{
		Unlock();
	}
}

void AMissionBarrier::Unlock()
{
	if (!bIsLocked)
	{
		return;
	}
	bIsLocked = false;

	if (Mesh)
	{
		Mesh->SetVisibility(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnBarrierUnlocked();
}
