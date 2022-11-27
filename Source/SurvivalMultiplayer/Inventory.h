// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "LineTrace.h"
#include "Components/ActorComponent.h"
#include "Inventory.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALMULTIPLAYER_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventory();

	bool AddItem(APickups* Item);

	UFUNCTION(BlueprintCallable)
		void DropItem(APickups* Item);

	bool CheckIfObjectHasItem(APickups* Item);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_DropItem(APickups* Item);
	bool Server_DropItem_Validate(APickups* Item);
	void Server_DropItem_Implementation(APickups* Item);

	UFUNCTION(BlueprintCallable)
		void UseItem(APickups* Item);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_UseItem(APickups* Item);
	bool Server_UseItem_Validate(APickups* Item);
	void Server_UseItem_Implementation(APickups* Item);

	void RemoveItemFromInventory(const APickups* Item);

	void DropAllItems();

	UFUNCTION(BlueprintCallable)
		int GetInventoryCount();

	UFUNCTION(BlueprintCallable)
		TArray<APickups*> GetInventoryItems();

	UFUNCTION(BlueprintCallable)
		void TransferItem(APickups* Item, AActor* Actor);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_TransferItem(APickups* Item, AActor* Actor);
	bool Server_TransferItem_Validate(APickups* Item, AActor* Actor);
	void Server_TransferItem_Implementation(APickups* Item, AActor* Actor);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void Server_ReceiveItem(APickups* Item);
	bool Server_ReceiveItem_Validate(APickups* Item);
	void Server_ReceiveItem_Implementation(APickups* Item);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TObjectPtr<ULineTrace> LineTrace;

	UPROPERTY(EditAnywhere)
		int InventoryCount;

	UPROPERTY(Replicated)
		TArray<TObjectPtr<APickups>> Items;
	
};
