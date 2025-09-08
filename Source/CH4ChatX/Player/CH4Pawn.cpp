// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CH4Pawn.h"
#include "CH4ChatX.h"

// Sets default values
ACH4Pawn::ACH4Pawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACH4Pawn::BeginPlay()
{
	Super::BeginPlay();

	FString NetRoleString = CH4ChatXFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::BeginPlay() %s [%s]"), *CH4ChatXFunctionLibrary::GetNetModeString(this), *NetRoleString);
	CH4ChatXFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
	
}

void ACH4Pawn::PossessedBy(AController* NewController)
{
	FString NetRoleString = CH4ChatXFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::PossessedBy() %s [%s]"), *CH4ChatXFunctionLibrary::GetNetModeString(this), *NetRoleString);
	CH4ChatXFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}
