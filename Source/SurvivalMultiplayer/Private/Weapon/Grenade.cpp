// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Grenade.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

AGrenade::AGrenade()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component"));
	SphereComponent->InitSphereRadius(305.0f);

	SphereComponent->SetupAttachment(MeshComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->UpdatedComponent = MeshComponent;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Friction = 0.1f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	ExplosionFX = CreateDefaultSubobject<UParticleSystemComponent>("Explosion FX");
	ExplosionFX->SetAutoActivate(false);
	ExplosionFX->SetupAttachment(MeshComponent);

	LineTrace = CreateDefaultSubobject<ULineTrace>(TEXT("LineTraceComponent"));

	FuseTime = 5.0f;
	ThrowSpeed = 1500.0f;
	bIsThrown = false;
}

void AGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGrenade, bIsThrown);
}

void AGrenade::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(Fuse_TimeHandle, this, &AGrenade::Explode, FuseTime);
	GetWorldTimerManager().PauseTimer(Fuse_TimeHandle);
}


bool AGrenade::IsHitValid(ASurvivalMultiplayerCharacter* Player)
{
	FHitResult HitResult = LineTrace->LineTraceSingle(this->GetActorLocation(), Player->GetActorLocation(), false);
	if(AActor* Actor = Cast<AActor>(HitResult.GetActor()))
	{
		if(Actor == Player)
			return true;
		else
			return false;
	}

	return false;
}

void AGrenade::OnRep_Explode()
{
	TArray<AActor*> OverlappingActors;
	SphereComponent->GetOverlappingActors(OverlappingActors);

	for(AActor* Actor: OverlappingActors)
	{
		if(ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(Actor))
		{
			if(IsHitValid(Character))
			{
				float Distance = FVector::Distance(this->GetActorLocation(), Character->GetActorLocation());
				int Damage = FMath::RoundToFloat((200.0f / Distance) * 100.0f);

				Character->TakeDamage(Damage, FDamageEvent(), nullptr, this);
			}
		}
	}
	
	ExplosionFX->Activate(true);
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, this->GetActorLocation());

	FTimerDelegate Delegate;
	FTimerHandle Destroy_TimerHandle;
	Delegate.BindLambda([this]
		{
			Destroy();
		});

	GetWorldTimerManager().SetTimer(Destroy_TimerHandle, Delegate, 2.0f, false);
}

void AGrenade::Explode()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Explode"));

		bIsThrown = true;
		OnRep_Explode();
	}
}

void AGrenade::Throw()
{
	if(HasAuthority())
	{
		GetWorldTimerManager().UnPauseTimer(Fuse_TimeHandle);

		if(this->GetOwner())
		{
			const ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(this->GetOwner());
			ProjectileMovement->Velocity = Character->GetControlRotation().Vector() * ThrowSpeed;
		}
	}
}
