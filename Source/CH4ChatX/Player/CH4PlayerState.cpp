// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CH4PlayerState.h"
#include "Net/UnrealNetwork.h"

ACH4PlayerState::ACH4PlayerState()
	: PlayerNameString(TEXT("None"))
	, CurrentGuessCount(0)
	, MaxGuessCount(3)
{
	bReplicates = true;
}

void ACH4PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
}

FString ACH4PlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = PlayerNameString + TEXT("(") + FString::FromInt(CurrentGuessCount + 1) + TEXT("/") + FString::FromInt(MaxGuessCount) + TEXT(")");
	return PlayerInfoString;
}
