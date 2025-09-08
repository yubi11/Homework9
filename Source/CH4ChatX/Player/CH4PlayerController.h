// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4PlayerController.generated.h"

class UCH4ChatInput;

UCLASS()
class CH4CHATX_API ACH4PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACH4PlayerController();

	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessageString);

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCH4ChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UCH4ChatInput> ChatInputWidgetInstance;

	FString ChatMessageString;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FText TurnTimerText;
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateTurnTime(float TimeRemaining, bool bIsMyTurn);

protected:
	// 턴 타이머 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TurnTimerWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TurnTimerWidgetInstance;
};
