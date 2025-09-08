// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CH4ChatInput.h"
#include "Components/EditableTextBox.h"
#include "Player/CH4PlayerController.h"

void UCH4ChatInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UCH4ChatInput::NativeDestruct()
{
	Super::NativeDestruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UCH4ChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* OwningPlayerController = GetOwningPlayer();
		if (IsValid(OwningPlayerController) == true)
		{
			ACH4PlayerController* OwningCH4PlayerController = Cast<ACH4PlayerController>(OwningPlayerController);
			if (IsValid(OwningCH4PlayerController) == true)
			{
				OwningCH4PlayerController->SetChatMessageString(Text.ToString());

				EditableTextBox_ChatInput->SetText(FText());
			}
		}
	}
}
