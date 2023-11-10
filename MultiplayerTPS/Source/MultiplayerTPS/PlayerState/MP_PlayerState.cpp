// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_PlayerState.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "Net/UnrealNetwork.h"

void AMP_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMP_PlayerState, Defeats);
	DOREPLIFETIME(AMP_PlayerState, Team);
}

void AMP_PlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMP_Character>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMP_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMP_PlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMP_Character>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMP_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMP_PlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AMP_Character>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMP_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AMP_PlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMP_Character>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMP_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AMP_PlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	AMP_Character* MP_Character = Cast<AMP_Character>(GetPawn());
	if (MP_Character)
	{
		MP_Character->SetTeamColor(Team);
	}
}

void AMP_PlayerState::OnRep_Team()
{
	AMP_Character* MP_Character = Cast<AMP_Character>(GetPawn());
	if (MP_Character)
	{
		MP_Character->SetTeamColor(Team);
	}
}