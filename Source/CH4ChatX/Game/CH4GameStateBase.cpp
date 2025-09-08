// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CH4GameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Player/CH4PlayerController.h"

void ACH4GameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ACH4PlayerController* CH4PC = Cast<ACH4PlayerController>(PC);
			if (IsValid(CH4PC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				CH4PC->PrintChatMessageString(NotificationString);
			}
		}
	}
}


