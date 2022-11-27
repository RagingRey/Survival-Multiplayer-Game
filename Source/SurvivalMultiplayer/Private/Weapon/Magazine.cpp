// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Magazine.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AMagazine::AMagazine()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = MeshComponent;

	bMagazineInUse = false;
}

// Called when the game starts or when spawned
void AMagazine::BeginPlay()
{
	Super::BeginPlay();

	SetReplicates(true);

	SetupMagazine(FName("SM_AR4_Mag"), false);
}

void AMagazine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagazine, MagazineAmmo);
	DOREPLIFETIME(AMagazine, bMagazineInUse);
}

void AMagazine::OnRep_MagazineInUse()
{
	this->MeshComponent->SetHiddenInGame(bMagazineInUse);
	this->SetActorEnableCollision(!bMagazineInUse);
}

void AMagazine::Pickup()
{
	bMagazineInUse = true;
	OnRep_MagazineInUse();
}

void AMagazine::SetupMagazine(FName MagazineName, bool bIsForWeapon)
{
	if (MagazineDataTable)
	{
		const FString ContextString = FName(MagazineName).ToString();
		MagazineData = MagazineDataTable->FindRow<FMagazineData>(MagazineName, ContextString, true);

		if (MagazineData)
		{
			MeshComponent->SetStaticMesh(MagazineData->StaticMesh);
			MagazineAmmo = MagazineData->MagazineCapacity;
			this->MeshComponent->SetHiddenInGame(bIsForWeapon);
			this->SetActorEnableCollision(!bIsForWeapon);
		}
	}
}

int AMagazine::CurrentAmmoCount()
{
	return MagazineAmmo;
}

void AMagazine::Fire()
{
	--MagazineAmmo;
}

TArray<FString> AMagazine::GetCompatibleWeapon()
{
	return MagazineData->CompatibleWeapons;
}
