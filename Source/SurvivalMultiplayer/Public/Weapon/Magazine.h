// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"

USTRUCT()
struct FMagazineData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere)
		FString MagazineName;

	UPROPERTY(EditAnywhere)
		TArray<FString> CompatibleWeapons;

	UPROPERTY(EditAnywhere)
		int MagazineCapacity;
};

UCLASS()
class SURVIVALMULTIPLAYER_API AMagazine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazine();

protected:

	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
		UDataTable* MagazineDataTable;

	FMagazineData* MagazineData;

	UPROPERTY(Replicated)
		int MagazineAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_MagazineInUse)
		bool bMagazineInUse;

	UFUNCTION()
		void OnRep_MagazineInUse();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void Pickup();
	int CurrentAmmoCount();
	void SetupMagazine(FName MagazineName, bool bIsForWeapon);
	void Fire();
	TArray<FString> GetCompatibleWeapon();
};
