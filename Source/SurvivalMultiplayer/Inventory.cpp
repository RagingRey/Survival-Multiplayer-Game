// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventory::UInventory()
{
	InventoryCount = 16;
}


// Called when the game starts
void UInventory::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
}

void UInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventory, Items);
}

bool UInventory::AddItem(APickups* Item)
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		Items.Add(Item);
		Item->InInventory(true);
	}
	
	return false;
}

void UInventory::DropAllItems()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		for (APickups* Item : Items)
		{
			DropItem(Item);
		}
	}
}

void UInventory::DropItem(APickups* Item)
{
	Server_DropItem(Item);
}

bool UInventory::CheckIfObjectHasItem(APickups* Item)
{
	for (const APickups* Pickup : Items)
	{
		if (Pickup == Item)
		{
			return true;
		}
	}

	return false;
}

bool UInventory::Server_DropItem_Validate(APickups* Item)
{
	return CheckIfObjectHasItem(Item);
}

void UInventory::Server_DropItem_Implementation(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FVector Location = this->GetOwner()->GetActorLocation();
		Location.X += FMath::RandRange(-50.0f, 100.0f);
		Location.Y += FMath::RandRange(-50.0f, 100.0f);

		FVector EndTrace = Location;
		EndTrace.Z -= 500.0f;

		FHitResult HitResult;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByObjectType(HitResult, Location, EndTrace, FCollisionObjectQueryParams::DefaultObjectQueryParam, CollisionQueryParams);

		if (HitResult.ImpactPoint != FVector::Zero())
			Location = HitResult.ImpactPoint;

		Item->SetActorLocation(Location);
		Item->InInventory(false);

		RemoveItemFromInventory(Item);
	}
}

void UInventory::UseItem(APickups* Item)
{
	Server_UseItem(Item);
}

bool UInventory::Server_UseItem_Validate(APickups* Item)
{
	return CheckIfObjectHasItem(Item);
}

void UInventory::Server_UseItem_Implementation(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if(ASurvivalMultiplayerCharacter* Player = Cast<ASurvivalMultiplayerCharacter>(GetOwner()))
		{
			Item->UseItem(Player);
			RemoveItemFromInventory(Item);
		}
	}
}

void UInventory::RemoveItemFromInventory(const APickups* Item)
{
	for(size_t i {0}; i < Items.Num(); ++i)
	{
		if(Items[i] == Item)
		{
			Items.RemoveAt(i);
		}
	}
}

TArray<APickups*> UInventory::GetInventoryItems()
{
	return Items;
}

void UInventory::TransferItem(APickups* Item, AActor* Actor)
{
	Server_TransferItem(Item, Actor);
}

bool UInventory::Server_TransferItem_Validate(APickups* Item, AActor* Actor)
{
	return CheckIfObjectHasItem(Item);
}

void UInventory::Server_TransferItem_Implementation(APickups* Item, AActor* Actor)
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		if (AStorage* Storage = Cast<AStorage>(Actor))
		{
			if (Item && Storage)
			{
				if (UInventory* StorageInventory = Storage->GetInventory())
				{
					StorageInventory->AddItem(Item);
					RemoveItemFromInventory(Item);
				}
			}
		}
		else if (ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(Actor))
		{
			if (Item && Character)
			{
				if (UInventory* PlayerInventory = Character->GetInventory())
				{
					PlayerInventory->AddItem(Item);
					RemoveItemFromInventory(Item);
				}
			}
		}
	}
}

bool UInventory::Server_ReceiveItem_Validate(APickups* Item)
{
	return true;
}

void UInventory::Server_ReceiveItem_Implementation(APickups* Item)
{
	if(ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(GetOwner()))
	{
		if(AStorage* Storage = Character->GetStorageComponent())
		{
			Storage->GetInventory()->TransferItem(Item, Character);
		}
	}
}

int UInventory::GetInventoryCount()
{
	return InventoryCount - 1;
}
