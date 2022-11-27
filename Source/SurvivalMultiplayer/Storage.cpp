// Fill out your copyright notice in the Description page of Project Settings.


#include "Storage.h"

#include "Inventory.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AStorage::AStorage()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	this->RootComponent = MeshComponent;

	Inventory = CreateDefaultSubobject<UInventory>(TEXT("Inventory"));
	IsOpened = false;
}

void AStorage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStorage, IsOpened);
}

// Called when the game starts or when spawned
void AStorage::BeginPlay()
{
	Super::BeginPlay();

	SetReplicates(true);
}

void AStorage::OpenChest(bool Open)
{
	if(GetLocalRole() == ROLE_Authority)
		Server_OpenClose(Open);
}

void AStorage::OnRep_Opened()
{
	if(IsOpened)
	{
		UE_LOG(LogTemp, Warning, TEXT("Animation"));
		MeshComponent->PlayAnimation(OpenAnimation, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Animation"));
		MeshComponent->PlayAnimation(CloseAnimation, false);
	}
}

bool AStorage::Server_OpenClose_Validate(bool Open)
{
	return true;
}

void AStorage::Server_OpenClose_Implementation(bool Open)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		IsOpened = Open;
	}
}

UInventory* AStorage::GetInventory()
{
	return Inventory;
}

bool AStorage::IsChestOpened()
{
	return IsOpened;
}