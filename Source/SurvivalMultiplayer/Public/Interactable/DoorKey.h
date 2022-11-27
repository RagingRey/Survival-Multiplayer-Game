// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurvivalMultiplayer/Pickups.h"
#include "DoorKey.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALMULTIPLAYER_API ADoorKey : public APickups
{
	GENERATED_BODY()

public:
	ADoorKey();

	UPROPERTY(EditAnywhere)
		class ADoor* LinkedDoor;

	class ADoor* GetLinkedDoor();
	
};
