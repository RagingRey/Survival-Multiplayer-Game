// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "SurvivalMultiplayer/SurvivalMultiplayerCharacter.h"
#include "Door.generated.h"

class ASurvivalMultiplayerCharacter;
UCLASS()
class SURVIVALMULTIPLAYER_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* StaticMesh;

	bool bIsLocked;
	
	UPROPERTY(EditAnywhere)
		FRotator OpenedRotation;
	FRotator ClosedRotation;

	FTimerHandle Door_TimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_ToggleDoor)
		bool bIsOpened;

	void RotateDoor();

	UFUNCTION()
		void OnRep_ToggleDoor();

	bool PlayerHasKey(ASurvivalMultiplayerCharacter* Player);
	bool CanInteract(ASurvivalMultiplayerCharacter* Player);

public:
	void ToggleDoor(ASurvivalMultiplayerCharacter* Player);
	void LockDoor(bool bLockDoor, ASurvivalMultiplayerCharacter* Player);
};
