// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureFlagGameMode.h"
#include "MultiplayerTPS/Weapon/Flag.h"
#include "MultiplayerTPS/CaptureTheFlag/FlagZone.h"
#include "MultiplayerTPS/GameState/MP_GameState.h"
#include "Kismet/GameplayStatics.h"

void ACaptureFlagGameMode::PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController)
{
	AMP_GameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
//	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AMP_GameState* MP_GameState = Cast<AMP_GameState>(GameState);
	if (MP_GameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			MP_GameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			MP_GameState->RedTeamScores();
		}
	}
}
