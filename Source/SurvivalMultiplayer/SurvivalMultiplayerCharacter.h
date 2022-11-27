// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Door.h"
#include "Storage.h"
#include "Component/ChatComponent.h"
#include "GameFramework/Character.h"
#include "Public/Weapon/WeaponBase.h"
#include "SurvivalMultiplayerCharacter.generated.h"

class AGrenade;

UCLASS(config=Game)
class ASurvivalMultiplayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ASurvivalMultiplayerCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

public:
	class UPlayerStatComponent* PlayerStatComp;
	class UInventory* Inventory;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		AStorage* GetStorageComponent();

	UFUNCTION(BlueprintCallable)
		UInventory* GetInventory();

	UFUNCTION(BlueprintCallable)
		UChatComponent* GetChatComponent();

protected:
	UPROPERTY(Replicated)
		float PlayerPitch;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_UpdatePitch(float Pitch);
	bool Server_UpdatePitch_Validate(float Pitch);
	void Server_UpdatePitch_Implementation(float Pitch);

	UFUNCTION(BlueprintCallable)
		float GetPlayerPitch();

	class ULineTrace* LineTraceComp;

	UChatComponent* ChatComponent;

	bool bIsSprinting;

	FTimerHandle SprintHandle;

	void HandleSprint();

	void Sprint();

	void StopSprinting();

	void Crouch();

	void StopCrouching();

	void AttemptJump();

	UPROPERTY(EditAnywhere)
		TSubclassOf<UUserWidget> InventoryWidget_Class;
	TObjectPtr<UUserWidget> InventoryWidget;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UUserWidget> DoorWidget_Class;
	TObjectPtr<UUserWidget> DoorWidget;

	UPROPERTY(ReplicatedUsing = OnRep_HandleStorageInventory)
		AStorage* StorageComponent;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AWeaponBase> Weapon_Class;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AGrenade> Grenade_Class;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponInteraction)
		TObjectPtr<AWeaponBase> Weapon;

	UFUNCTION()
		void OnRep_WeaponInteraction();

	UFUNCTION()
		void OnRep_HandleStorageInventory();

	void HandleInventory();

	bool bDoubleClicked;
	FTimerHandle Interact_TimerHandle;

	ADoor* InteractedDoor;

	void DoubleInteract();
	void CheckDoubleInteract();
	void SingleInteract();

	void Interact(bool bWasDoubleClicked);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Interact(FVector End);
	bool Server_Interact_Validate(FVector End);
	void Server_Interact_Implementation(FVector End);

	UFUNCTION(BlueprintCallable)
		void LockDoor(bool bLock);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_LockDoor(bool bLock, ADoor* Door);
	bool Server_LockDoor_Validate(bool bLock, ADoor* Door);
	void Server_LockDoor_Implementation(bool bLock, ADoor* Door);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Aim)
		bool bIsAiming;

	void Aim();
	void StopAiming();

	UFUNCTION()
		void OnRep_Aim();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_HandleAim(bool IsAiming);
	bool Server_HandleAim_Validate(bool IsAiming);
	void Server_HandleAim_Implementation(bool IsAiming);

	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Attack(FHitResult HitResult);
	bool Server_Attack_Validate(FHitResult HitResult);
	void Server_Attack_Implementation(FHitResult HitResult);

	void ThrowGrenade();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ThrowGrenade(FVector ThrowLocation, FRotator ThrowRotation);
	bool Server_ThrowGrenade_Validate(FVector ThrowLocation, FRotator ThrowRotation);
	void Server_ThrowGrenade_Implementation(FVector ThrowLocation, FRotator ThrowRotation);

	void Reload();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Reload();
	bool Server_Reload_Validate();
	void Server_Reload_Implementation();

	UPROPERTY(EditAnywhere)
		UAnimationAsset* DeathAnimation;

	void Die();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_Die();
	bool Multi_Die_Validate();
	void Multi_Die_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CloseInventory();
	bool Server_CloseInventory_Validate();
	void Server_CloseInventory_Implementation();

	UFUNCTION(BlueprintPure)
		FString ReturnPlayerStat();

	virtual void BeginPlay() override;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

