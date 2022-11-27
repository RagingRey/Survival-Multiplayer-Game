// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SurvivalMultiplayer/Pickups.h"
#include "Grenade.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALMULTIPLAYER_API AGrenade : public APickups
{
	GENERATED_BODY()

public:
	AGrenade();

protected:
	
	UPROPERTY(EditAnywhere)
		TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UParticleSystemComponent> ExplosionFX;

	UPROPERTY(EditAnywhere)
		TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditAnywhere)
		int FuseTime;

	UPROPERTY(EditAnywhere)
		float ThrowSpeed;

	FTimerHandle Fuse_TimeHandle;

	UPROPERTY(ReplicatedUsing = OnRep_Explode)
		bool bIsThrown;

	ULineTrace* LineTrace;

	bool IsHitValid(ASurvivalMultiplayerCharacter* Player);

	UFUNCTION()
		void OnRep_Explode();

	void Explode();

	virtual void BeginPlay() override;

public:
	void Throw();
};
