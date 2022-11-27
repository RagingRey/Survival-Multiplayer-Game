// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SpawnPoint.h"
#include "SurvivalMultiplayerCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalMultiplayerGameMode.generated.h"

UCLASS(minimalapi)
class ASurvivalMultiplayerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASurvivalMultiplayerGameMode();

	void Respawn(AController* Controller);

protected:
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly, Category = Respawn)
		TSubclassOf<ASpawnPoint> SpawnPoint_Class;
	TArray<AActor*> SpawnPoints_Actors;
	TArray<ASpawnPoint*> SpawnPoints;

	ASpawnPoint* GetSpawnPoint();

	UPROPERTY(EditAnywhere, Category = Respawn)
		float RespawnTime;

	FVector DefaultSpawnLocation = FVector(1500.0, 1870.0, 250.0);

	UFUNCTION()
		void Spawn(AController* Controller);
};



