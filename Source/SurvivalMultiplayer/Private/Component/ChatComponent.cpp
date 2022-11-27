// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/ChatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SurvivalMultiplayerPlayerState.h"
#include "SurvivalMultiplayer/SurvivalMultiplayerCharacter.h"

// Sets default values for this component's properties
UChatComponent::UChatComponent()
{
	MinimumTimeBetweenMessages = 3;
}


// Called when the game starts
void UChatComponent::BeginPlay()
{
	Super::BeginPlay();

	Time = FDateTime::Now();
}

void UChatComponent::Client_ReceiveMessage_Implementation(const FText& Message)
{
	if (!Message.IsEmpty()) 
	{
		ChatReceived.Broadcast(Message);
	}
}

bool UChatComponent::IsValidTimeToSendMessage()
{
	FDateTime CurrentTime = FDateTime::Now();

	const int CurrentSeconds = (CurrentTime.GetDay() * 86400) + (CurrentTime.GetHour() * 3600) + (CurrentTime.GetMinute() * 60) + CurrentTime.GetSecond();
	const int PreviousSeconds = (Time.GetDay() * 86400) + (Time.GetHour() * 3600) + (Time.GetMinute() * 60) + Time.GetSecond();

	return CurrentSeconds > (PreviousSeconds + MinimumTimeBetweenMessages);
}

void UChatComponent::SendMessage(FText Message)
{
	if(!IsValidTimeToSendMessage())
		return;

	if(Message.ToString().Len() > 0)
	{
		Server_SendMessage(Message);
		Time = FDateTime::Now();
	}
}

bool UChatComponent::Server_SendMessage_Validate(const FText& Message)
{
	return true;
}

void UChatComponent::Server_SendMessage_Implementation(const FText& Message)
{
	if (!IsValidTimeToSendMessage())
		return;

	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurvivalMultiplayerCharacter::StaticClass(), AllCharacters);

	for (AActor* Actor : AllCharacters) 
	{
		if (ASurvivalMultiplayerCharacter* Character = Cast<ASurvivalMultiplayerCharacter>(Actor))
		{
			if(ASurvivalMultiplayerPlayerState* PlayerState = Cast<ASurvivalMultiplayerPlayerState>(Character->GetPlayerState()))
			{
				FString MessageToSend = PlayerState->GetPlayerName() + ": " + Message.ToString();
				Character->GetChatComponent()->Client_ReceiveMessage(FText::FromString(MessageToSend));
			}
		}
	}
}


TArray<FText> UChatComponent::GetChatMessages()
{
	return TArray<FText>();
}

FText UChatComponent::ValidMessage(const FText& Message)
{
	FString Temp = Message.ToString();

	if (Temp.Len() > 120)
		Temp = Temp.Mid(0, 120);

	return FText::FromString(Temp);
}
