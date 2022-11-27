// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalMultiplayerCharacter.h"

#include "Door.h"
#include "Inventory.h"
#include "LineTrace.h"
#include "Pickups.h"
#include "PlayerStatComponent.h"
#include "Storage.h"
#include "SurvivalMultiplayerGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Grenade.h"
#include "Weapon/Magazine.h"

//////////////////////////////////////////////////////////////////////////
// ASurvivalMultiplayerCharacter

class ASurvivalMultiplayerGameMode;

ASurvivalMultiplayerCharacter::ASurvivalMultiplayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.0f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	PlayerStatComp = CreateDefaultSubobject<UPlayerStatComponent>(TEXT("PlayerStatComponent"));
	Inventory = CreateDefaultSubobject<UInventory>(TEXT("Invetory"));
	LineTraceComp = CreateDefaultSubobject<ULineTrace>(TEXT("LineTraceComponent"));
	ChatComponent = CreateDefaultSubobject<UChatComponent>(TEXT("Chat Component"));

	DeathAnimation = CreateDefaultSubobject<UAnimationAsset>(TEXT("Death Animation"));

	bIsSprinting = false;
	bIsAiming = false;
	InteractedDoor = nullptr;
}

void ASurvivalMultiplayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	/*if(Weapon_Class)
	{
		FTransform WeaponTransform;
		WeaponTransform.SetLocation(FVector::Zero());
		WeaponTransform.SetRotation(FQuat(FRotator::ZeroRotator));

		Weapon = GetWorld()->SpawnActor<AWeaponBase>(Weapon_Class, WeaponTransform);

		if(Weapon)
		{
			Weapon->SetActorEnableCollision(false);
			Weapon->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("GunPoint"));
		}
	}

	GetWorldTimerManager().SetTimer(SprintHandle, this, &ASurvivalMultiplayerCharacter::HandleSprint, 1.0f, true);*/
}

void ASurvivalMultiplayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASurvivalMultiplayerCharacter, StorageComponent, COND_OwnerOnly);
	DOREPLIFETIME(ASurvivalMultiplayerCharacter, Weapon);
	DOREPLIFETIME(ASurvivalMultiplayerCharacter, bIsAiming);
	DOREPLIFETIME_CONDITION(ASurvivalMultiplayerCharacter, PlayerPitch, COND_SkipOwner);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASurvivalMultiplayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASurvivalMultiplayerCharacter::AttemptJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASurvivalMultiplayerCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASurvivalMultiplayerCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalMultiplayerCharacter::Crouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalMultiplayerCharacter::StopCrouching);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASurvivalMultiplayerCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASurvivalMultiplayerCharacter::StopAiming);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalMultiplayerCharacter::SingleInteract);
	PlayerInputComponent->BindAction("Interact", IE_DoubleClick, this, &ASurvivalMultiplayerCharacter::DoubleInteract);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASurvivalMultiplayerCharacter::Attack);
	PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &ASurvivalMultiplayerCharacter::ThrowGrenade);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASurvivalMultiplayerCharacter::Reload);

	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &ASurvivalMultiplayerCharacter::HandleInventory);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ASurvivalMultiplayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ASurvivalMultiplayerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ASurvivalMultiplayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ASurvivalMultiplayerCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASurvivalMultiplayerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASurvivalMultiplayerCharacter::TouchStopped);
}

void ASurvivalMultiplayerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ASurvivalMultiplayerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ASurvivalMultiplayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ASurvivalMultiplayerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());

	const FRotator NormalizedRotation = (GetControlRotation() - GetActorRotation()).GetNormalized();
	PlayerPitch = NormalizedRotation.Pitch;

	Server_UpdatePitch(PlayerPitch);
}

bool ASurvivalMultiplayerCharacter::Server_UpdatePitch_Validate(float Pitch)
{
	return true;
}
void ASurvivalMultiplayerCharacter::Server_UpdatePitch_Implementation(float Pitch)
{
	PlayerPitch = Pitch;
}

float ASurvivalMultiplayerCharacter::GetPlayerPitch()
{
	if(PlayerPitch > 180.0f)
	{
		return 360.0f - PlayerPitch;
	}
	return PlayerPitch * -1;
}


void ASurvivalMultiplayerCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (!bIsSprinting)
			Value /= 2;

		AddMovementInput(Direction, Value);
	}
}

void ASurvivalMultiplayerCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction

		if (!bIsSprinting)
			Value /= 2;

		AddMovementInput(Direction, Value);
	}
}

void ASurvivalMultiplayerCharacter::HandleSprint()
{
	if(bIsSprinting && this->GetVelocity().Size())
	{
		PlayerStatComp->ControlSprintingTimer(true);
		PlayerStatComp->LowerStamina(2.0f);

		if(PlayerStatComp->GetStamina() <= 10.0f)
		{
			StopSprinting();
		}
	}
}

void ASurvivalMultiplayerCharacter::Sprint()
{
	if(PlayerStatComp->GetStamina() > 10.0f)
	{
		bIsSprinting = true;
	}
}

void ASurvivalMultiplayerCharacter::StopSprinting()
{
	bIsSprinting = false;
	PlayerStatComp->ControlSprintingTimer(false);
}

void ASurvivalMultiplayerCharacter::Crouch()
{
	if(!GetMovementComponent()->IsCrouching() && !GetMovementComponent()->IsFalling())
	{
		GetCharacterMovement()->bWantsToCrouch = true;
		GetCharacterMovement()->MaxWalkSpeed = 250.f;
	}
}

void ASurvivalMultiplayerCharacter::StopCrouching()
{
	if (GetMovementComponent()->IsCrouching())
	{
		GetCharacterMovement()->bWantsToCrouch = false;
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
	}
}

void ASurvivalMultiplayerCharacter::AttemptJump()
{
	if(PlayerStatComp->GetStamina() > 10.0f && !GetMovementComponent()->IsFalling())
	{
		Jump();
		PlayerStatComp->LowerStamina(10.0f);
	}
}

void ASurvivalMultiplayerCharacter::HandleInventory()
{
	if(InventoryWidget_Class)
	{
		APlayerController* PlayerController = Cast<APlayerController>(this->GetController());
		if(InventoryWidget && InventoryWidget->IsInViewport())
		{
			InventoryWidget->RemoveFromParent();
			PlayerController->SetShowMouseCursor(false);
			PlayerController->SetInputMode(FInputModeGameOnly());
			if (StorageComponent)
				Server_CloseInventory();
		}
		else
		{
			InventoryWidget = CreateWidget(GetWorld(), InventoryWidget_Class);
			InventoryWidget->AddToViewport();
			PlayerController->SetShowMouseCursor(true);
			PlayerController->SetInputMode(FInputModeGameAndUI());
		} 
	}
}


void ASurvivalMultiplayerCharacter::OnRep_HandleStorageInventory()
{
	if (InventoryWidget_Class)
	{
		if(InventoryWidget && InventoryWidget->IsInViewport())
			InventoryWidget->RemoveFromParent();

		APlayerController* PlayerController = Cast<APlayerController>(this->GetController());
		if (!StorageComponent)
		{
			InventoryWidget->RemoveFromParent();
			PlayerController->SetShowMouseCursor(false);
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
		else
		{
			InventoryWidget = CreateWidget(GetWorld(), InventoryWidget_Class);
			InventoryWidget->AddToViewport();
			PlayerController->SetShowMouseCursor(true);
			PlayerController->SetInputMode(FInputModeGameAndUI());
		}
	}
}

void ASurvivalMultiplayerCharacter::DoubleInteract()
{
	bDoubleClicked = true;
	Interact(true);
}

void ASurvivalMultiplayerCharacter::CheckDoubleInteract()
{
	if (bDoubleClicked)
		bDoubleClicked = false;
	else
		Interact(false);
}

void ASurvivalMultiplayerCharacter::SingleInteract()
{
	GetWorldTimerManager().SetTimer(Interact_TimerHandle, this, &ASurvivalMultiplayerCharacter::CheckDoubleInteract, 0.15f, false);
}

void ASurvivalMultiplayerCharacter::Interact(bool bWasDoubleClicked)
{
	FVector CamStart = this->FollowCamera->GetComponentLocation();
	FVector CamEnd = CamStart + FollowCamera->GetForwardVector() * 2000.0f;
	FVector ImpactPoint = LineTraceComp->LineTraceSingle(CamStart, CamEnd, false).ImpactPoint;


	FVector Start = this->GetMesh()->GetBoneLocation(FName("head"));
	FVector End = Start + UKismetMathLibrary::FindLookAtRotation(Start, ImpactPoint).Vector() * 2000.0f;

	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);

	if (Cast<APickups>(HitResult.GetActor()))
	{
		Server_Interact(End);
	}
	else if (Cast<AStorage>(HitResult.GetActor()))
	{
		Server_Interact(End);
	}
	else if (Cast<AWeaponBase>(HitResult.GetActor()))
	{
		Server_Interact(End);
	}
	else if (ADoor* Door = Cast<ADoor>(HitResult.GetActor()))
	{
		InteractedDoor = Door;

		if(bWasDoubleClicked && DoorWidget_Class)
		{
			APlayerController* PlayerController = Cast<APlayerController>(this->GetController());
			if (DoorWidget && DoorWidget->IsInViewport())
			{
				DoorWidget->RemoveFromParent();
				PlayerController->SetShowMouseCursor(false);
				PlayerController->SetInputMode(FInputModeGameOnly());
			}
			else
			{
				DoorWidget = CreateWidget(GetWorld(), DoorWidget_Class);
				DoorWidget->AddToViewport();
				PlayerController->SetShowMouseCursor(true);
				PlayerController->SetInputMode(FInputModeGameAndUI());
			}
		}
		else
		{
			Server_Interact(End);
		}
	}
	else if (AMagazine* Magazine = Cast<AMagazine>(HitResult.GetActor()))
	{
		if(Weapon && Weapon->IsMagazineCompatible(Magazine))
			Server_Interact(End);
	}
}

bool ASurvivalMultiplayerCharacter::Server_Interact_Validate(FVector End)
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_Interact_Implementation(FVector End)
{
	if(HasAuthority())
	{
		FVector Start = this->GetMesh()->GetBoneLocation(FName("head"));

		FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);

		if (APickups* Pickup = Cast<APickups>(HitResult.GetActor()))
		{
			Inventory->AddItem(Pickup);
			UE_LOG(LogTemp, Warning, TEXT("Hit"));
			//Pickup->UseItem(this);
		}
		else if(AStorage* Storage = Cast<AStorage>(HitResult.GetActor()))
		{
			if (Storage->IsChestOpened() && !StorageComponent)
				return;

			bool Open = false;
			if (StorageComponent)
			{
				StorageComponent = nullptr;
			}
			else
			{
				Open = true;
				StorageComponent = Storage;
			}
			Storage->OpenChest(Open);
		}
		else if (AWeaponBase* WeaponObject = Cast<AWeaponBase>(HitResult.GetActor()))
		{
			Weapon = WeaponObject;
			Weapon->SetOwner(this);
			OnRep_WeaponInteraction();
		}
		else if (ADoor* Door = Cast<ADoor>(HitResult.GetActor()))
		{
			Door->ToggleDoor(this);
		}
		else if (AMagazine* Magazine = Cast<AMagazine>(HitResult.GetActor()))
		{
			Weapon->AddMagazine(Magazine);
		}
	}
}

void ASurvivalMultiplayerCharacter::LockDoor(bool bLock)
{
	if(InteractedDoor)
	{
		Server_LockDoor(bLock, InteractedDoor);
	}
}

bool ASurvivalMultiplayerCharacter::Server_LockDoor_Validate(bool bLock, ADoor* Door)
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_LockDoor_Implementation(bool bLock, ADoor* Door)
{
	Door->LockDoor(bLock, this);
}

void ASurvivalMultiplayerCharacter::Aim()
{
	if(Weapon)
		Server_HandleAim(true);
}

void ASurvivalMultiplayerCharacter::StopAiming()
{
	if (Weapon)
		Server_HandleAim(false);
}

void ASurvivalMultiplayerCharacter::OnRep_Aim()
{
	bUseControllerRotationYaw = bIsAiming;
}

bool ASurvivalMultiplayerCharacter::Server_HandleAim_Validate(bool IsAiming)
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_HandleAim_Implementation(bool IsAiming)
{
	bIsAiming = IsAiming;
	OnRep_Aim();
}

void ASurvivalMultiplayerCharacter::OnRep_WeaponInteraction()
{
	Weapon->SetActorEnableCollision(false);
	Weapon->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("GunPoint"));
}

bool ASurvivalMultiplayerCharacter::Server_CloseInventory_Validate()
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_CloseInventory_Implementation()
{
	if(StorageComponent)
	{
		StorageComponent->OpenChest(false);
		StorageComponent = nullptr;
	}
}


void ASurvivalMultiplayerCharacter::Attack()
{
	if (Weapon)
	{
		if (bIsAiming)
		{
			FVector CamStart = this->FollowCamera->GetComponentLocation();
			FVector CamEnd = CamStart + FollowCamera->GetForwardVector() * 3500.0f;
			FVector ImpactPoint = LineTraceComp->LineTraceSingle(CamStart, CamEnd, false).ImpactPoint;

			Server_Attack(Weapon->Fire(ImpactPoint));
		}
		else
		{
			Server_Attack(Weapon->Fire());
		}
	}
}

bool ASurvivalMultiplayerCharacter::Server_Attack_Validate(FHitResult HitResult)
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_Attack_Implementation(FHitResult HitResult)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		Weapon->Fire(HitResult);
	}
}

void ASurvivalMultiplayerCharacter::ThrowGrenade()
{
	if (Inventory)
	{
		for (APickups* Pickup : Inventory->GetInventoryItems())
		{
			if (AGrenade* Grenade = Cast<AGrenade>(Pickup))
			{
				UE_LOG(LogTemp, Warning, TEXT("Thrown"));
				Server_ThrowGrenade(this->GetMesh()->GetBoneLocation(FName("head")), this->GetControlRotation());
				return;
			}
		}
	}
}

bool ASurvivalMultiplayerCharacter::Server_ThrowGrenade_Validate(FVector ThrowLocation, FRotator ThrowRotation)
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_ThrowGrenade_Implementation(FVector ThrowLocation, FRotator ThrowRotation)
{
	if (Inventory)
	{
		for (APickups* Pickup : Inventory->GetInventoryItems())
		{
			if (AGrenade* Grenade = Cast<AGrenade>(Pickup))
			{
				if (Grenade_Class)
				{
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.Owner = this;
					SpawnParameters.bNoFail = true;

					ThrowLocation += ThrowRotation.Vector() * 100.0f;

					AGrenade* NewGrenade = GetWorld()->SpawnActor<AGrenade>(Grenade_Class, ThrowLocation, this->GetActorRotation(), SpawnParameters);
					NewGrenade->Throw();

					Inventory->RemoveItemFromInventory(Grenade);
					Grenade->Destroy();
					return;
				}
			}
		}
	}
}


void ASurvivalMultiplayerCharacter::Reload()
{
	if(Weapon && Weapon->CanReload())
	{
		Server_Reload();
	}
}

bool ASurvivalMultiplayerCharacter::Server_Reload_Validate()
{
	return true;
}

void ASurvivalMultiplayerCharacter::Server_Reload_Implementation()
{
	Weapon->Reload();
}

float ASurvivalMultiplayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                                AController* EventInstigator, AActor* DamageCauser)
{
	if (GetLocalRole() < ROLE_Authority || PlayerStatComp->GetHealth() <= 0.0)
		return 0.0f;

	const float ActualDamage =  Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if(ActualDamage > 0.0f)
	{
		PlayerStatComp->LowerHealth(ActualDamage);

		if(PlayerStatComp->GetHealth() <= 0.0)
		{
			Die();
		}
	}

	return ActualDamage;
}

void ASurvivalMultiplayerCharacter::Die()
{
	if(GetLocalRole() == ROLE_Authority)
	{
		Server_CloseInventory();
		Inventory->DropAllItems();
		Multi_Die();

		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
		if (ASurvivalMultiplayerGameMode* Mode = Cast<ASurvivalMultiplayerGameMode>(GameMode))
		{
			Mode->Respawn(GetController());
		}

		FTimerHandle DeathHandle;
		FTimerDelegate Delegate;

		Delegate.BindLambda([&]
			{
				Destroy();
			});

		GetWorldTimerManager().SetTimer(DeathHandle, Delegate, 10.0f, false);
	}
}

bool ASurvivalMultiplayerCharacter::Multi_Die_Validate()
{
	return true;
}

void ASurvivalMultiplayerCharacter::Multi_Die_Implementation()
{
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh()->PlayAnimation(DeathAnimation, false);
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh()->SetSimulatePhysics(true);
}

FString ASurvivalMultiplayerCharacter::ReturnPlayerStat()
{
	return "Hunger: " + FString::SanitizeFloat(PlayerStatComp->GetHunger())
		+ "         Thirst: " + FString::SanitizeFloat(PlayerStatComp->GetThirst())
		+ "        Stamina: " + FString::SanitizeFloat(PlayerStatComp->GetStamina())
		+ "        Health: " + FString::SanitizeFloat(PlayerStatComp->GetHealth());
}

UInventory* ASurvivalMultiplayerCharacter::GetInventory()
{
	return Inventory;
}

UChatComponent* ASurvivalMultiplayerCharacter::GetChatComponent()
{
	return ChatComponent;
}


AStorage* ASurvivalMultiplayerCharacter::GetStorageComponent()
{
	return StorageComponent;
}
