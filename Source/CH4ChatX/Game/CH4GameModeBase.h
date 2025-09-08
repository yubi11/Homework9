// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CH4GameModeBase.generated.h"

class ACH4PlayerController;

UCLASS()
class CH4CHATX_API ACH4GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	virtual void BeginPlay() override;

	void PrintChatMessageString(ACH4PlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void IncreaseGuessCount(ACH4PlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ACH4PlayerController* InChattingPlayerController, int InStrikeCount);

	void CheckForDraw();

protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ACH4PlayerController>> AllPlayerControllers;

	// �� ���� ����
	int32 CurrentTurnPlayerIndex = 0;
	float TurnTimeLimit = 30.0f;
	float CurrentTurnTimeRemaining = 0.0f;

	// �÷��̾ �� ���� ���� ����
	TMap<ACH4PlayerController*, bool> PlayerHasGuessedThisTurn;

	// Ÿ�̸� �ڵ�
	FTimerHandle TurnTimerHandle;

public:
	// �� ���� �Լ�
	void StartNewTurn();
	void EndCurrentTurn();
	void AdvanceToNextPlayer();
	void HandleTurnTimeout();

	// �ð� ������Ʈ �� ��ε�ĳ��Ʈ
	void UpdateTurnTime();
	void BroadcastTurnTimeToClients(float TimeRemaining);
};
