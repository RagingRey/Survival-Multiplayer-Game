// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatComponent.h"

#include "SurvivalMultiplayerCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UPlayerStatComponent::UPlayerStatComponent()
{
	Hunger = 100.0f;
	Thirst = 100.0f;

	HungerDecrementValue = 0.3f;
	ThirstDecrementValue = 0.5f;

	Stamina = 20.0f;
	Health = 30.0f;
}


// Called when the game starts
void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);

	if (GetOwnerRole() == ROLE_Authority) 
	{
		GetWorld()->GetTimerManager().SetTimer(HungerAndThirstHandle, this, &UPlayerStatComponent::HandleHungerAndThirst, 3.0f, true);
		GetWorld()->GetTimerManager().SetTimer(StaminaHandle, this, &UPlayerStatComponent::RegenerateStamina, 1.0f, true);
	}
}

void UPlayerStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Hunger, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Thirst, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Stamina, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Health, COND_OwnerOnly);
}

void UPlayerStatComponent::HandleHungerAndThirst()
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(HungerDecrementValue);
		LowerThirst(ThirstDecrementValue);
	}
}

void UPlayerStatComponent::LowerHunger(float Value)
{
	if(GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerHunger(Value);
	}
	else
	{
		Hunger -= Value;

		if(Hunger < 0.0f)
		{
			if(ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(GetOwner()))
			{
				Character->TakeDamage(Health * -1, FDamageEvent(), Character->GetController(), Character);
			}
		}
	}
}

void UPlayerStatComponent::LowerThirst(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerThirst(Value);
	}
	else
	{
		Thirst -= Value;

		if (Thirst < 0.0f)
		{
			if (ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(GetOwner()))
			{
				Character->TakeDamage(Thirst * -1, FDamageEvent(), Character->GetController(), Character);
			}
		}
	}
}

void UPlayerStatComponent::LowerStamina(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerStamina(Value);
	}
	else
	{
		Stamina -= Value;
	}
}

void UPlayerStatComponent::LowerHealth(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerHealth(Value);
	}
	else
	{
		Health -= Value;

		if (Health < 0.0f)
			Health = 0.0f;
	}
}

bool UPlayerStatComponent::ServerLowerHunger_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerHunger_Implementation(float Value)
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(Value);
	}
}

bool UPlayerStatComponent::ServerLowerThirst_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerThirst_Implementation(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerThirst(Value);
	}
}

bool UPlayerStatComponent::ServerLowerStamina_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerStamina_Implementation(float Value)
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		LowerStamina(Value);
	}
}

void UPlayerStatComponent::ControlSprintingTimer(bool IsSprinting)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerControlSprintingTimer_Implementation(IsSprinting);
	}
	else
	{
		if (IsSprinting)
			GetWorld()->GetTimerManager().PauseTimer(StaminaHandle);
		else
			GetWorld()->GetTimerManager().UnPauseTimer(StaminaHandle);
	}
}

bool UPlayerStatComponent::ServerControlSprintingTimer_Validate(bool IsSprinting)
{
	return true;
}

void UPlayerStatComponent::ServerControlSprintingTimer_Implementation(bool IsSprinting)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ControlSprintingTimer(IsSprinting);
	}
}

bool UPlayerStatComponent::ServerLowerHealth_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerHealth_Implementation(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHealth(Value);
	}
}

void UPlayerStatComponent::AddHunger(float Value)
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		if ((Hunger + Value) > 100.0f)
			Hunger = 100.0f;
		else
			Hunger += Value;
	}
}

void UPlayerStatComponent::AddThirst(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if ((Thirst + Value) > 100.0f)
			Thirst = 100.0f;
		else
			Thirst += Value;
	}
}

void UPlayerStatComponent::AddHealth(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if ((Health + Value) > 100.0f)
			Health = 100.0f;
		else
			Health += Value;
	}
}

float UPlayerStatComponent::GetHunger()
{
	return Hunger;
}

void UPlayerStatComponent::RegenerateStamina()
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		if (Stamina == 100.0f)
			Stamina = 100.0f;
		else
			++Stamina;
	}
}

float UPlayerStatComponent::GetThirst()
{
	return Thirst;
}

float UPlayerStatComponent::GetStamina()
{
	return Stamina;
}

float UPlayerStatComponent::GetHealth()
{
	return Health;
}
