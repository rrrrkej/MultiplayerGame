// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_GameState.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"

void AMP_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMP_GameState, TopScoringPlayers);
	DOREPLIFETIME(AMP_GameState, RedTeamScore);
	DOREPLIFETIME(AMP_GameState, BlueTeamScore);
}

void AMP_GameState::UpdateTopScore(AMP_PlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AMP_GameState::OnRep_RedTeamScore()
{
	AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMP_GameState::OnRep_BlueTeamScore()
{
	AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AMP_GameState::RedTeamScores()
{
	RedTeamScore++;
	AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMP_GameState::BlueTeamScores()
{
	BlueTeamScore++;
	AMP_PlayerController* MP_PlayerController = Cast<AMP_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
