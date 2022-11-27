// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurvivalMultiplayerCharacter.h"
#include "GameFramework/Actor.h"
#include "Pickups.generated.h"

UENUM()
enum class EPickUpType : uint8
{
	Food,
	Water,
	MedKit,
	DoorKey,
	Grenade
};

UCLASS()
class SURVIVALMULTIPLAYER_API APickups : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickups();

	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, Category = Enums)
		EPickUpType PickUp;

	UPROPERTY(ReplicatedUsing = OnRep_PickedUp)
		bool OnPickedUp;

	UFUNCTION()
		void OnRep_PickedUp();

protected:
	float IncreaseValue;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void UseItem(ASurvivalMultiplayerCharacter* Character);
	void InInventory(bool In);

	UFUNCTION(BlueprintCallable)
		UTexture2D* GetIcon();
};
