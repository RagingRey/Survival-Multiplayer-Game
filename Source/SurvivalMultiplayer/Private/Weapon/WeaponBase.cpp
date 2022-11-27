// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalMultiplayer/Public/Weapon/WeaponBase.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "SurvivalMultiplayer/SurvivalMultiplayerCharacter.h"
#include "Weapon/Magazine.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("Skeletal Mesh");
	RootComponent = MeshComponent;

	LineTrace = CreateDefaultSubobject<ULineTrace>(TEXT("LineTraceComponent"));

	DefaultWeaponName = "";
	bMagazineSpawned = false;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	SetReplicates(true);

	if (DefaultWeaponName.GetStringLength() != 0)
		SetupWeapon(DefaultWeaponName);
	
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, CurrentMagazine);
	DOREPLIFETIME(AWeaponBase, ExtraMagazine);
	DOREPLIFETIME(AWeaponBase, bMagazineSpawned);
}

bool AWeaponBase::IsHitValid(FHitResult ClientHitResult, FHitResult ServerHitResult)
{

	float ClientStart = ClientHitResult.TraceStart.Size();
	float ClientEnd = ClientHitResult.TraceEnd.Size();
	float ServerStart = ServerHitResult.TraceStart.Size();
	float ServerEnd = ServerHitResult.TraceEnd.Size();

	if(ClientStart >= (ServerStart - 15) && ClientStart <= (ServerStart + 15) && ClientEnd >= (ServerEnd - 15) && ClientEnd <= (ServerEnd + 15))
	{
		UE_LOG(LogTemp, Warning, TEXT("Valid"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid"));
		return false;	
	}
}

void AWeaponBase::SetupWeapon(FName WeaponName)
{
	if (WeaponDataTable)
	{
		const FString ContextString = FName(WeaponName).ToString();
		WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, ContextString, true);

		if (WeaponData)
		{
			MeshComponent->SetSkeletalMesh(WeaponData->SkeletalMesh);
			if (GetLocalRole() == ROLE_Authority)
			{
				if (!WeaponData->MagazineName.IsEmpty() && WeaponData->Magazine_Class)
				{
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.bNoFail = true;
					SpawnParameters.Owner = this;

					AMagazine* Magazine = GetWorld()->SpawnActor<AMagazine>(WeaponData->Magazine_Class, this->GetActorLocation(), this->GetActorRotation(), SpawnParameters);
					CurrentMagazine = Magazine;

					const size_t RandomMagazineCount = FMath::RandRange(1, 3);
					for (size_t i{ 0 }; i < RandomMagazineCount; ++i)
					{
						AMagazine* EMagazine = GetWorld()->SpawnActor<AMagazine>(WeaponData->Magazine_Class, this->GetActorLocation(), this->GetActorRotation(), SpawnParameters);
						ExtraMagazine.Add(EMagazine);
					}

					bMagazineSpawned = true;
					OnRep_MagazineSpawned();
				}
			}
		}
	}
}

void AWeaponBase::MagSpawned()
{
}

void AWeaponBase::OnRep_MagazineSpawned()
{
	if(CurrentMagazine)
	{
		CurrentMagazine->SetupMagazine(WeaponData->MagazineName[0], false);
	}

	if(!ExtraMagazine.IsEmpty())
	{
		for(AMagazine* Magazine: ExtraMagazine)
		{
			if(Magazine)
				Magazine->SetupMagazine(WeaponData->MagazineName[0], false);
		}
	}
}

FHitResult AWeaponBase::Fire()
{
	if (GetLocalRole() < ROLE_Authority && CurrentMagazine)
	{
		if (CurrentMagazine->CurrentAmmoCount() > 0)
		{
			if (WeaponData && WeaponData->FireAnimation)
			{
				MeshComponent->PlayAnimation(WeaponData->FireAnimation, false);
			}

			FVector StartLocation = this->MeshComponent->GetSocketLocation(FName("muzzle"));
			FRotator Rotation = this->MeshComponent->GetSocketRotation(FName("muzzle"));
			FVector EndPosition = StartLocation + Rotation.Vector() * 3500.0f;

			CurrentMagazine->Fire();
			UE_LOG(LogTemp, Warning, TEXT("Remaining Ammo: %d"), CurrentMagazine->CurrentAmmoCount())
				return LineTrace->LineTraceSingle(StartLocation, EndPosition, true);
		}
		else
		{
			return FHitResult();
		}
	}

	return FHitResult();
}

FHitResult AWeaponBase::Fire(FVector End)
{
	if(GetLocalRole() < ROLE_Authority && CurrentMagazine)
	{
		if (CurrentMagazine->CurrentAmmoCount() > 0)
		{
			if (WeaponData && WeaponData->FireAnimation)
			{
				MeshComponent->PlayAnimation(WeaponData->FireAnimation, false);
			}

			FVector StartLocation = this->MeshComponent->GetSocketLocation(FName("muzzle"));
			FRotator Rotation = this->MeshComponent->GetSocketRotation(FName("muzzle"));
			FVector EndPosition = StartLocation + UKismetMathLibrary::FindLookAtRotation(StartLocation, End).Vector() * 3500.0f;

			CurrentMagazine->Fire();
			UE_LOG(LogTemp, Warning, TEXT("Remaining Ammo: %d"), CurrentMagazine->CurrentAmmoCount())
			return LineTrace->LineTraceSingle(StartLocation, EndPosition, true);
		}
		else
		{
			return FHitResult();
		}
	}

	return FHitResult();
}

FHitResult AWeaponBase::Fire(FHitResult ClientHitResult)
{
	if (GetLocalRole() == ROLE_Authority && CurrentMagazine)
	{
		if (CurrentMagazine->CurrentAmmoCount() > 0)
		{
			FVector StartLocation = this->MeshComponent->GetSocketLocation(FName("muzzle"));

			if (AActor* Actor = ClientHitResult.GetActor())
			{
				FRotator Rotation = this->MeshComponent->GetSocketRotation(FName("muzzle"));
				FVector EndPosition = StartLocation + UKismetMathLibrary::FindLookAtRotation(StartLocation, ClientHitResult.TraceEnd).Vector() * 3500.0f;

				FHitResult ServerHitResult = LineTrace->LineTraceSingle(StartLocation, EndPosition, true);
				CurrentMagazine->Fire();

				if (IsHitValid(ClientHitResult, ServerHitResult))
				{
					if (ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(Actor))
					{
						float Damage = 20.0f;
						Character->TakeDamage(Damage, FDamageEvent(), nullptr, GetOwner());
					}
				}
			}
		}
		else
		{
			return FHitResult();
		}
	}

	return FHitResult();
}

bool AWeaponBase::CanReload()
{
	return (ExtraMagazine.Num() > 0);
}

void AWeaponBase::Reload()
{
	AMagazine* MagazineTemp = CurrentMagazine;

	AMagazine* FullestMag = ExtraMagazine[0];
	size_t FullestMagIndex = 0;
	for(UPTRINT i {0}; i < ExtraMagazine.Num(); i++)
	{
		if(ExtraMagazine[i]->CurrentAmmoCount() > FullestMag->CurrentAmmoCount())
		{
			FullestMag = ExtraMagazine[i];
			FullestMagIndex = i;
		}
	}

	CurrentMagazine = FullestMag;
	ExtraMagazine.RemoveAt(FullestMagIndex);

	if(MagazineTemp->CurrentAmmoCount() > 0)
		ExtraMagazine.Add(MagazineTemp);

	for(AMagazine* Mag: ExtraMagazine)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ammos: %d"), Mag->CurrentAmmoCount());
	}
}

bool AWeaponBase::IsMagazineCompatible(AMagazine* Magazine)
{
	if(!Magazine->GetCompatibleWeapon().IsEmpty())
	{
		for(size_t i {0}; i < Magazine->GetCompatibleWeapon().Num(); ++i)
		{
			if(Magazine->GetCompatibleWeapon()[i] == WeaponData->WeaponName)
			{
				return true;
			}
		}
	}

	return false;
}

void AWeaponBase::AddMagazine(AMagazine* Magazine)
{
	ExtraMagazine.Add(Magazine);
	Magazine->Pickup();
}
