// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "SurvivalMultiplayer/LineTrace.h"
#include "WeaponBase.generated.h"

USTRUCT()
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere)
		FString WeaponName;

	UPROPERTY(EditAnywhere)
		UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
		TArray<FName> MagazineName;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AMagazine> Magazine_Class;
};

UCLASS()
class SURVIVALMULTIPLAYER_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UDataTable> WeaponDataTable;

	FWeaponData* WeaponData;

	ULineTrace* LineTrace;

	UPROPERTY(EditAnywhere)
		FName DefaultWeaponName;

	UPROPERTY(Replicated)
		class AMagazine* CurrentMagazine;

	UPROPERTY(Replicated)
		TArray<TObjectPtr<class AMagazine>> ExtraMagazine;

	UPROPERTY(ReplicatedUsing = OnRep_MagazineSpawned)
		bool bMagazineSpawned;

	void MagSpawned();

	UFUNCTION()
		void OnRep_MagazineSpawned();

	bool IsHitValid(FHitResult ClientHitResult, FHitResult ServerHitResult);

	void SetupWeapon(FName WeaponName);

public:
	FHitResult Fire();
	FHitResult Fire(FVector End);
	FHitResult Fire(FHitResult ClientHitResult);
	bool CanReload();
	void Reload();
	bool IsMagazineCompatible(AMagazine* Magazine);
	void AddMagazine(AMagazine* Magazine);
};
