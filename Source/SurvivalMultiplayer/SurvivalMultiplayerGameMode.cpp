// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalMultiplayerGameMode.h"
#include "SurvivalMultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ASurvivalMultiplayerGameMode::ASurvivalMultiplayerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	RespawnTime = 3.0f;
}

void ASurvivalMultiplayerGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnPoint_Class)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnPoint_Class, SpawnPoints_Actors);

		for (int i{ 0 }; i < SpawnPoints_Actors.Num(); i++)
		{
			SpawnPoints.Add(Cast<ASpawnPoint>(SpawnPoints_Actors[i]));
		}
	}
}

void ASurvivalMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if(AController* Controller = Cast<AController>(NewPlayer))
	{
		Spawn(Controller);
	}
}

ASpawnPoint* ASurvivalMultiplayerGameMode::GetSpawnPoint()
{
	if (SpawnPoints.Num() > 0)
	{
		int Slot = FMath::RandRange(0, SpawnPoints.Num() - 1);
		return SpawnPoints[Slot];
	}

	return nullptr;
}

void ASurvivalMultiplayerGameMode::Spawn(AController* Controller)
{
	if (Controller)
	{
		if (ASpawnPoint* SpawnPoint = GetSpawnPoint())
		{
			FVector Location(SpawnPoint->GetActorLocation());
			FRotator Rotation(SpawnPoint->GetActorRotation());

			if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotation))
			{
				Controller->Possess(Pawn);
			}
		}
		else
		{
			if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, DefaultSpawnLocation, FRotator::ZeroRotator))
			{
				Controller->Possess(Pawn);
			}
		}
	}
}

void ASurvivalMultiplayerGameMode::Respawn(AController* Controller)
{
	FTimerHandle RespawnHandle;
	FTimerDelegate SpawnDelegate;
	SpawnDelegate.BindUFunction(this, FName("Spawn"), Controller);
	GetWorldTimerManager().SetTimer(RespawnHandle, SpawnDelegate, RespawnTime, false);
}