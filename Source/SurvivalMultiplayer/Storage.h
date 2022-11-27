// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Storage.generated.h"

UCLASS()
class SURVIVALMULTIPLAYER_API AStorage : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStorage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere)
		class UInventory* Inventory;

	UPROPERTY(EditAnywhere)
		class UAnimationAsset* OpenAnimation;

	UPROPERTY(EditAnywhere)
		class UAnimationAsset* CloseAnimation;

	UPROPERTY(ReplicatedUsing = OnRep_Opened)
		bool IsOpened;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_OpenClose(bool Open);
	bool Server_OpenClose_Validate(bool Open);
	void Server_OpenClose_Implementation(bool Open);

public:
	UFUNCTION(BlueprintCallable)
		class UInventory* GetInventory();

	UFUNCTION()
		void OnRep_Opened();

	void OpenChest(bool Open);

	bool IsChestOpened();
};
