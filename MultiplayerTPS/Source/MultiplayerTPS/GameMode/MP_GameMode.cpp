﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_GameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"
#include "MultiplayerTPS/GameState/MP_GameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

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
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}

	// update variable
	UpdateSecond += DeltaTime;
	if (UpdateSecond > 1.f)
	{
		UpdateGlobalPing();
		UpdateSecond = 0;
	}
}

void AMP_GameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(*It);
		if (MP_PlayerController)
		{
			MP_PlayerController->OnMatchStateSet(MatchState, bTeamsMode);
		}
	}
}

void AMP_GameMode::UpdateGlobalPing()
{
	if (GetWorld()->GetNumPlayerControllers() == 1) return;	// 如果只有服务器一个玩家

	float sum = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(*It);
		if (MP_PlayerController)
		{
			if (MP_PlayerController->IsLocalController()) continue;
			sum += MP_PlayerController->AveragePing;
		}
	}
	GlobalPing = sum / (GetWorld()->GetNumPlayerControllers() - 1);
}


float AMP_GameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void AMP_GameMode::PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AMP_PlayerState* AttackerPlayerState = AttackerController ? Cast<AMP_PlayerState>(AttackerController->PlayerState) : nullptr;
	AMP_PlayerState* VictimPlayerState = VictimController ? Cast<AMP_PlayerState>(VictimController->PlayerState) : nullptr;

	AMP_GameState* MP_GameState = GetGameState<AMP_GameState>();

	// Update AttackerPlayerState
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && MP_GameState)
	{
		// save current leader
		TArray<AMP_PlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : MP_GameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		// score calculation
		AttackerPlayerState->AddToScore(1.f);
		MP_GameState->UpdateTopScore(AttackerPlayerState);
		if (MP_GameState->TopScoringPlayers.Contains(AttackerPlayerState)) // Niagara for Leader
		{
			AMP_Character* Leader = Cast<AMP_Character>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}
		// Deactive Niagara for Loser
		for (AMP_PlayerState* Player : PlayersCurrentlyInTheLead)
		{
			if (!MP_GameState->TopScoringPlayers.Contains(Player))
			{
				AMP_Character* Loser = Cast<AMP_Character>(Player->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	// Update VictimPlayerState
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	// Elim victim character
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	// Broadcast elim announcement
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMP_PlayerController* MP_Player = Cast<AMP_PlayerController>(*It);
		if (MP_Player && AttackerPlayerState && VictimPlayerState)
		{
			MP_Player->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
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

void AMP_GameMode::PlayerLeftGame(AMP_PlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;

	//	delete PlayerLeaving if on point rank 
	AMP_GameState* MP_GameState = GetGameState<AMP_GameState>();
	if (MP_GameState && MP_GameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		MP_GameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	//
	AMP_Character* CharacterLeaving = Cast<AMP_Character>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

