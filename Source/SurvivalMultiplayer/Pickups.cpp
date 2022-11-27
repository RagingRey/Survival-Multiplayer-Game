// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups.h"

#include "PlayerStatComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APickups::APickups()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	RootComponent = MeshComponent;

	Icon = CreateDefaultSubobject<UTexture2D>("Icon");

	IncreaseValue = 30.0f;
	OnPickedUp = false;
}

// Called when the game starts or when spawned
void APickups::BeginPlay()
{
	Super::BeginPlay();

	SetReplicates(true);
	SetReplicateMovement(true);
}

void APickups::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickups, OnPickedUp);
}

void APickups::OnRep_PickedUp()
{
	this->MeshComponent->SetHiddenInGame(OnPickedUp);
	this->SetActorEnableCollision(!OnPickedUp);
	
}

void APickups::UseItem(ASurvivalMultiplayerCharacter* Character)
{
	if (GetLocalRole() == ROLE_Authority && PickUp != EPickUpType::DoorKey && PickUp != EPickUpType::Grenade)
	{
		if(PickUp == EPickUpType::Food)
		{
			UE_LOG(LogTemp, Warning, TEXT("Adding Hunger"));
			Character->PlayerStatComp->AddHunger(IncreaseValue);
		}
		else if(PickUp == EPickUpType::Water)
		{
			UE_LOG(LogTemp, Warning, TEXT("Adding Thirst"));
			Character->PlayerStatComp->AddThirst(IncreaseValue);
		}
		else if (PickUp == EPickUpType::MedKit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Adding Health"));
			Character->PlayerStatComp->AddHealth(IncreaseValue);
		}
		this->Destroy();
	}
}
	
void APickups::InInventory(bool In)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		OnPickedUp = In;
		OnRep_PickedUp();
	}
}

UTexture2D* APickups::GetIcon()
{
	return Icon;
}
