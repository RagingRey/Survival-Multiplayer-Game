// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

#include "Interactable/DoorKey.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "SurvivalMultiplayer/Inventory.h"
#include "SurvivalMultiplayer/Pickups.h"

// Sets default values
ADoor::ADoor()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	this->RootComponent = StaticMesh;

	bIsOpened = false;
	bIsLocked = true;
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();

	SetReplicates(true);

	ClosedRotation = this->GetActorRotation();

	GetWorldTimerManager().SetTimer(Door_TimerHandle, this, &ADoor::RotateDoor, 0.02, true);
	GetWorldTimerManager().PauseTimer(Door_TimerHandle);
}

void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, bIsOpened);
}

void ADoor::RotateDoor()
{
	if(bIsOpened)
	{
		FRotator CurrentRotation = this->GetActorRotation();
		FRotator NewRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, OpenedRotation, GetWorld()->GetDeltaSeconds(), 7.0f);
		this->SetActorRotation(NewRotation);
		if(NewRotation.Equals(OpenedRotation, 0.1))
		{
			GetWorldTimerManager().PauseTimer(Door_TimerHandle);
		}
	}
	else
	{
		FRotator CurrentRotation = this->GetActorRotation();
		FRotator NewRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, ClosedRotation, GetWorld()->GetDeltaSeconds(), 7.0f);
		this->SetActorRotation(NewRotation);
		if (NewRotation.Equals(ClosedRotation, 0.1))
		{
			GetWorldTimerManager().PauseTimer(Door_TimerHandle);
		}
	}
}

void ADoor::OnRep_ToggleDoor()
{
	GetWorldTimerManager().UnPauseTimer(Door_TimerHandle);
}

bool ADoor::PlayerHasKey(ASurvivalMultiplayerCharacter* Player)
{
	if (UInventory* PlayerInventory = Player->GetInventory())
	{
		for (APickups* Item : PlayerInventory->GetInventoryItems())
		{
			if (ADoorKey* Key = Cast<ADoorKey>(Item))
			{
				if (this == Key->GetLinkedDoor())
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool ADoor::CanInteract(ASurvivalMultiplayerCharacter* Player)
{
	if(PlayerHasKey(Player) && this->GetDistanceTo(Player) <= 200.f)
	{
		return true;
	}

	return false;
}

void ADoor::ToggleDoor(ASurvivalMultiplayerCharacter* Player)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		if(bIsOpened)
		{
			bIsOpened = !bIsOpened;
			OnRep_ToggleDoor();
			return;
		}
		if(bIsLocked)
		{
			if (PlayerHasKey(Player))
			{
				bIsLocked = false;
				bIsOpened = !bIsOpened;
				OnRep_ToggleDoor();
			}
		}
		else
		{
			bIsOpened = !bIsOpened;
			OnRep_ToggleDoor();
		}
	}
}

void ADoor::LockDoor(bool bLockDoor, ASurvivalMultiplayerCharacter* Player)
{
	if(GetLocalRole() == ROLE_Authority && CanInteract(Player))
		bIsLocked = bLockDoor;
}
