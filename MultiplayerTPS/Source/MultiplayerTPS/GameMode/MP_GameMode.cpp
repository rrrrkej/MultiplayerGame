// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_GameMode.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AMP_GameMode::PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController)
{
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
