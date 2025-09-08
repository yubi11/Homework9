// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CH4PlayerController.h"
#include "UI/CH4ChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CH4ChatX.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/CH4GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Player/CH4PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "Blueprint/UserWidget.h"


ACH4PlayerController::ACH4PlayerController()
{
	bReplicates = true;
}

void ACH4PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass))
	{
		ChatInputWidgetInstance = CreateWidget<UCH4ChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance))
		{
			ChatInputWidgetInstance->AddToViewport();
			bShowMouseCursor = true;
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}

	// 턴 타이머 위젯 생성
	if (IsValid(TurnTimerWidgetClass))
	{
		TurnTimerWidgetInstance = CreateWidget<UUserWidget>(this, TurnTimerWidgetClass);
		if (IsValid(TurnTimerWidgetInstance))
		{
			TurnTimerWidgetInstance->AddToViewport();
		}
	}

}

void ACH4PlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	/*
	//PrintChatMessageString(ChatMessageString);
	if (IsLocalController() == true)
	{
		ServerRPCPrintChatMessageString(InChatMessageString);
	}*/

	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		// ServerRPCPrintChatMessageString(InChatMessageString);

		ACH4PlayerState* CH4PS = GetPlayerState<ACH4PlayerState>();
		if (IsValid(CH4PS) == true)
		{
			//FString CombinedMessageString = CH4PS->PlayerNameString + TEXT(": ") + InChatMessageString;
			FString CombinedMessageString = CH4PS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;
			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void ACH4PlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);
	//디버깅 로그 제거 시 아래 두 줄 주석 처리
	//FString NetModeString = CH4ChatXFunctionLibrary::GetNetModeString(this);
	//FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	CH4ChatXFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ACH4PlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
	DOREPLIFETIME(ThisClass, TurnTimerText);
}

void ACH4PlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ACH4PlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	/*
	for (TActorIterator<ACH4PlayerController> It(GetWorld()); It; ++It)
	{
		ACH4PlayerController* CH4PlayerController = *It;
		if (IsValid(CH4PlayerController) == true)
		{
			CH4PlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}*/

	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ACH4GameModeBase* CH4GM = Cast<ACH4GameModeBase>(GM);
		if (IsValid(CH4GM) == true)
		{
			CH4GM->PrintChatMessageString(this, InChatMessageString);
		}
	}

}

void ACH4PlayerController::ClientUpdateTurnTime_Implementation(float TimeRemaining, bool bIsMyTurn)
{
	FString TimeString = FString::Printf(TEXT("Time: %.0f"), TimeRemaining);

	if (bIsMyTurn)
	{
		TimeString += TEXT(" (Your Turn!)");
	}
	else
	{
		TimeString += TEXT(" (Waiting...)");
	}

	TurnTimerText = FText::FromString(TimeString);
}