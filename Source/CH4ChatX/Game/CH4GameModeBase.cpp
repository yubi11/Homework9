// Fill out your copyright notice in the Description page of Project Settings.


#include "CH4GameModeBase.h"
#include "CH4GameStateBase.h"
#include "Player/CH4PlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Player/CH4PlayerState.h"

void ACH4GameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ACH4PlayerController* CH4PlayerController = Cast<ACH4PlayerController>(NewPlayer);
	if (IsValid(CH4PlayerController) == true)
	{
		CH4PlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(CH4PlayerController);

		ACH4PlayerState* CH4PS = CH4PlayerController->GetPlayerState<ACH4PlayerState>();
		if (IsValid(CH4PS) == true)
		{
			CH4PS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		// 첫 번째 플레이어 접속 시 게임 시작
		if (AllPlayerControllers.Num() == 1)
		{
			// 약간의 지연 후 게임 시작 (다른 플레이어들의 접속을 위해)
			FTimerHandle StartGameTimer;
			GetWorld()->GetTimerManager().SetTimer(StartGameTimer, this, &ACH4GameModeBase::StartNewTurn, 2.0f, false);
		}

		ACH4GameStateBase* CH4GameStateBase = GetGameState<ACH4GameStateBase>();
		if (IsValid(CH4GameStateBase) == true)
		{
			CH4GameStateBase->MulticastRPCBroadcastLoginMessage(CH4PS->PlayerNameString);
		}
	}
}

void ACH4GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("SecretNumberString: %s"), *SecretNumberString);

}

FString ACH4GameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ACH4GameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}

FString ACH4GameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ACH4GameModeBase::PrintChatMessageString(ACH4PlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 현재 턴 플레이어인지 확인
	if (!AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex) ||
		InChattingPlayerController != AllPlayerControllers[CurrentTurnPlayerIndex])
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("It's not your turn!"));
		return;
	}

	// 플레이어의 남은 기회 확인
	ACH4PlayerState* PS = InChattingPlayerController->GetPlayerState<ACH4PlayerState>();
	if (!IsValid(PS) || PS->CurrentGuessCount >= PS->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("You have no more chances left!"));
		EndCurrentTurn();
		return;
	}

	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		// 플레이어가 이번 턴에 추측했음을 표시
		PlayerHasGuessedThisTurn[InChattingPlayerController] = true;

		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		IncreaseGuessCount(InChattingPlayerController);

		for (TActorIterator<ACH4PlayerController> It(GetWorld()); It; ++It)
		{
			ACH4PlayerController* CH4PlayerController = *It;
			if (IsValid(CH4PlayerController) == true)
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				CH4PlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);

				int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
				JudgeGame(InChattingPlayerController, StrikeCount);
			}
		}

		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));

		if (StrikeCount == 3)
		{
			JudgeGame(InChattingPlayerController, StrikeCount);
			return;
		}

		// 턴 종료 후 무승부 체크
		EndCurrentTurn();
		CheckForDraw();
	}
	else
	{
		for (TActorIterator<ACH4PlayerController> It(GetWorld()); It; ++It)
		{
			ACH4PlayerController* CH4PlayerController = *It;
			if (IsValid(CH4PlayerController) == true)
			{
				CH4PlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void ACH4GameModeBase::IncreaseGuessCount(ACH4PlayerController* InChattingPlayerController)
{
	ACH4PlayerState* CH4PS = InChattingPlayerController->GetPlayerState<ACH4PlayerState>();
	if (IsValid(CH4PS) == true)
	{
		CH4PS->CurrentGuessCount++;
	}
}

void ACH4GameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("New SecretNumberString: %s"), *SecretNumberString);

	for (const auto& CH4PlayerController : AllPlayerControllers)
	{
		ACH4PlayerState* CH4PS = CH4PlayerController->GetPlayerState<ACH4PlayerState>();
		if (IsValid(CH4PS) == true)
		{
			CH4PS->CurrentGuessCount = 0;
		}

		// 모든 플레이어의 NotificationText 초기화
		CH4PlayerController->NotificationText = FText::FromString(TEXT("Game Reset! New round starting..."));
	}

	// 턴 정보 리셋
	CurrentTurnPlayerIndex = 0;
	PlayerHasGuessedThisTurn.Empty();
	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	// 1초 후 새 게임 시작 (리셋 메시지를 볼 시간 제공)
	FTimerHandle StartGameTimer;
	GetWorld()->GetTimerManager().SetTimer(StartGameTimer, this, &ACH4GameModeBase::StartNewTurn, 1.0f, false);
}

void ACH4GameModeBase::JudgeGame(ACH4PlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

		ACH4PlayerState* CH4PS = InChattingPlayerController->GetPlayerState<ACH4PlayerState>();
		for (const auto& CH4PlayerController : AllPlayerControllers)
		{
			if (IsValid(CH4PS) == true)
			{
				FString CombinedMessageString = CH4PS->PlayerNameString + TEXT(" has won the game.");
				CH4PlayerController->NotificationText = FText::FromString(CombinedMessageString);

			}
		}
		// 3초 후 게임 리셋
		FTimerHandle ResetTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetTimer, this, &ACH4GameModeBase::ResetGame, 3.0f, false);
		return;
	}
	// 무승부 조건 체크: 모든 플레이어가 3번 다 시도했는지 확인
	CheckForDraw();
}

// 무승부 체크를 별도 함수로 분리
void ACH4GameModeBase::CheckForDraw()
{
	bool bAllPlayersExhausted = true;

	for (const auto& PlayerController : AllPlayerControllers)
	{
		ACH4PlayerState* PS = PlayerController->GetPlayerState<ACH4PlayerState>();
		if (IsValid(PS) && PS->CurrentGuessCount < PS->MaxGuessCount)
		{
			bAllPlayersExhausted = false;
			break;
		}
	}

	if (bAllPlayersExhausted)
	{
		// 타이머 정지
		GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

		for (const auto& PlayerController : AllPlayerControllers)
		{
			PlayerController->NotificationText = FText::FromString(TEXT("Draw... No one guessed the number!"));
		}

		// 3초 후 게임 리셋
		FTimerHandle ResetTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetTimer, this, &ACH4GameModeBase::ResetGame, 3.0f, false);
	}
}

void ACH4GameModeBase::StartNewTurn()
{
	if (AllPlayerControllers.Num() == 0) return;

	// 현재 턴 플레이어 설정
	CurrentTurnTimeRemaining = TurnTimeLimit;

	// 모든 플레이어의 턴 진행 상태 초기화
	for (auto& PlayerController : AllPlayerControllers)
	{
		PlayerHasGuessedThisTurn.Add(PlayerController, false);
	}

	// 모든 플레이어에게 상태 메시지 업데이트
	if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex))
	{
		ACH4PlayerController* CurrentPlayer = AllPlayerControllers[CurrentTurnPlayerIndex];
		ACH4PlayerState* CurrentPS = CurrentPlayer ? CurrentPlayer->GetPlayerState<ACH4PlayerState>() : nullptr;

		for (int32 i = 0; i < AllPlayerControllers.Num(); ++i)
		{
			ACH4PlayerController* PlayerController = AllPlayerControllers[i];
			if (IsValid(PlayerController))
			{
				if (i == CurrentTurnPlayerIndex)
				{
					// 현재 턴 플레이어
					PlayerController->NotificationText = FText::FromString(TEXT("Your turn! Make your guess."));
				}
				else
				{
					// 다른 플레이어들
					if (IsValid(CurrentPS))
					{
						FString WaitMessage = FString::Printf(TEXT("Waiting for %s's turn..."), *CurrentPS->PlayerNameString);
						PlayerController->NotificationText = FText::FromString(WaitMessage);
					}
					else
					{
						PlayerController->NotificationText = FText::FromString(TEXT("Waiting for other player's turn..."));
					}
				}
			}
		}
	}

	// 타이머 시작 (1초마다 업데이트)
	GetWorld()->GetTimerManager().SetTimer(
		TurnTimerHandle,
		this,
		&ACH4GameModeBase::UpdateTurnTime,
		1.0f,
		true
	);

	// 모든 클라이언트에 턴 시작 알림
	BroadcastTurnTimeToClients(CurrentTurnTimeRemaining);
}

void ACH4GameModeBase::UpdateTurnTime()
{
	CurrentTurnTimeRemaining -= 1.0f;

	// 시간 업데이트를 클라이언트들에게 전송
	BroadcastTurnTimeToClients(CurrentTurnTimeRemaining);

	if (CurrentTurnTimeRemaining <= 0.0f)
	{
		HandleTurnTimeout();
	}
}

void ACH4GameModeBase::HandleTurnTimeout()
{
	// 타이머 정지
	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex))
	{
		ACH4PlayerController* CurrentPlayer = AllPlayerControllers[CurrentTurnPlayerIndex];
		ACH4PlayerState* CurrentPS = CurrentPlayer ? CurrentPlayer->GetPlayerState<ACH4PlayerState>() : nullptr;

		// 플레이어가 이번 턴에 추측하지 않았으면 기회 소진
		if (IsValid(CurrentPlayer) &&
			(!PlayerHasGuessedThisTurn.Contains(CurrentPlayer) ||
				!PlayerHasGuessedThisTurn[CurrentPlayer]))
		{
			IncreaseGuessCount(CurrentPlayer);

			// 모든 플레이어에게 타임아웃 메시지 브로드캐스트
			for (auto& PlayerController : AllPlayerControllers)
			{
				if (PlayerController == CurrentPlayer)
				{
					PlayerController->NotificationText = FText::FromString(TEXT("Time's up! You lost a chance."));
				}
				else if (IsValid(CurrentPS))
				{
					FString TimeoutMessage = FString::Printf(TEXT("%s's time is up!"), *CurrentPS->PlayerNameString);
					PlayerController->NotificationText = FText::FromString(TimeoutMessage);
				}
			}
		}
	}

	EndCurrentTurn();
}

void ACH4GameModeBase::BroadcastTurnTimeToClients(float TimeRemaining)
{
	if (!AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex)) return;

	ACH4PlayerController* CurrentTurnPlayer = AllPlayerControllers[CurrentTurnPlayerIndex];

	for (auto& PlayerController : AllPlayerControllers)
	{
		if (IsValid(PlayerController))
		{
			// 현재 턴 플레이어인지 여부를 직접 전달
			bool bIsMyTurn = (PlayerController == CurrentTurnPlayer);
			PlayerController->ClientUpdateTurnTime(TimeRemaining, bIsMyTurn);
		}
	}
}

void ACH4GameModeBase::EndCurrentTurn()
{
	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);

	// 먼저 무승부 체크
	bool bAllPlayersExhausted = true;
	for (auto& PlayerController : AllPlayerControllers)
	{
		ACH4PlayerState* PS = PlayerController->GetPlayerState<ACH4PlayerState>();
		if (IsValid(PS) && PS->CurrentGuessCount < PS->MaxGuessCount)
		{
			bAllPlayersExhausted = false;  // 기회가 남은 플레이어가 있으면 false
			break;
		}
	}

	// 모든 플레이어가 기회를 다 썼으면 무승부 처리
	if (bAllPlayersExhausted)
	{
		UE_LOG(LogTemp, Warning, TEXT("All players exhausted - Draw!"));

		for (const auto& PlayerController : AllPlayerControllers)
		{
			PlayerController->NotificationText = FText::FromString(TEXT("Draw... No one guessed the number!"));
		}

		// 3초 후 게임 리셋
		FTimerHandle ResetTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetTimer, this, &ACH4GameModeBase::ResetGame, 3.0f, false);
		return;
	}

	// 게임이 계속되면 다음 턴 시작
	AdvanceToNextPlayer();
	StartNewTurn();
}

void ACH4GameModeBase::AdvanceToNextPlayer()
{

	int32 StartIndex = CurrentTurnPlayerIndex;
	int32 CheckCount = 0;

	do {
		CurrentTurnPlayerIndex = (CurrentTurnPlayerIndex + 1) % AllPlayerControllers.Num();
		CheckCount++;

		if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex))
		{
			ACH4PlayerController* NextPlayer = AllPlayerControllers[CurrentTurnPlayerIndex];
			ACH4PlayerState* PS = NextPlayer->GetPlayerState<ACH4PlayerState>();

			// 아직 기회가 남은 플레이어면 턴 넘김
			if (IsValid(PS) && PS->CurrentGuessCount < PS->MaxGuessCount)
			{
				UE_LOG(LogTemp, Warning, TEXT("Next turn: Player %s"), *PS->PlayerNameString);
				return;
			}
		}

		// 무한루프 방지: 모든 플레이어를 한 번씩 확인했으면 중단
		if (CheckCount >= AllPlayerControllers.Num())
		{
			UE_LOG(LogTemp, Warning, TEXT("No players with remaining chances found"));
			return;
		}

	} while (CurrentTurnPlayerIndex != StartIndex);
}