// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_GameMode.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"

AMP_GameMode::AMP_GameMode()
{
	bDelayedStart = true;
}

void AMP_GameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMP_GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
}

void AMP_GameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	if (MatchState == MatchState::WaitingToStart)
	{

	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(*It);
		if (MP_PlayerController)
		{
			MP_PlayerController->OnMatchStateSet(MatchState);
		}
	}
}

void AMP_GameMode::PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController)
{
	AMP_PlayerState* AttackerPlayerState = AttackerController ? Cast<AMP_PlayerState>(AttackerController->PlayerState) : nullptr;
	AMP_PlayerState* VictimPlayerState = VictimController ? Cast<AMP_PlayerState>(VictimController->PlayerState) : nullptr;

	// Update AttackerPlayerState
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	// Update VictimPlayerState
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	// Elim victim character
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void AMP_GameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

