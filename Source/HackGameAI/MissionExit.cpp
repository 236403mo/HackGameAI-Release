// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionExit.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MissionGameMode.h"
#include "DestructibleTarget.h"

AMissionExit::AMissionExit()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;

	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	PadMesh->SetupAttachment(RootComponent);
	PadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMissionExit::BeginPlay()
{
	// bIsActive must be resolved before Super::BeginPlay(), because AActor::BeginPlay()
	// invokes the Blueprint's EventBeginPlay as part of the same call, and the Blueprint
	// needs the correct value to set the pad's initial material.
	if (RequiredTarget)
	{
		bIsActive = false;
		RequiredTarget->OnTargetDestroyed.AddDynamic(this, &AMissionExit::HandleRequiredTargetDestroyed);
	}
	else
	{
		bIsActive = true;
	}

	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMissionExit::HandleOverlap);
}

void AMissionExit::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered || !bIsActive)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!OtherActor || OtherActor != PlayerPawn)
	{
		return;
	}

	bHasTriggered = true;

	if (AMissionGameMode* GameMode = Cast<AMissionGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->CompleteMission();
	}
}

void AMissionExit::HandleRequiredTargetDestroyed()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	OnExitActivated();
}
