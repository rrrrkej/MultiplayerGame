// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "MultiplayerTPS/GameState/MP_GameState.h"
#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 团队对抗，实现两边人数平衡
	AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
	if (MP_GameState)
	{
		AMP_PlayerState* MP_PlayerState = NewPlayer->GetPlayerState<AMP_PlayerState>();
		if (MP_PlayerState && MP_PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (MP_GameState->BlueTeam.Num() >= MP_GameState->RedTeam.Num())
			{
				MP_GameState->RedTeam.AddUnique(MP_PlayerState);
				MP_PlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else if (MP_GameState->BlueTeam.Num() < MP_GameState->RedTeam.Num())
			{
				MP_GameState->BlueTeam.AddUnique(MP_PlayerState);
				MP_PlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	// 退出房间注销其在队伍中的数据
	AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
	AMP_PlayerState* MP_PlayerState = Exiting->GetPlayerState<AMP_PlayerState>();
	if (MP_GameState && MP_PlayerState)
	{
		if (MP_GameState->RedTeam.Contains(MP_PlayerState))
		{
			MP_GameState->RedTeam.Remove(MP_PlayerState);
		}
		if (MP_GameState->BlueTeam.Contains(MP_PlayerState))
		{
			MP_GameState->BlueTeam.Remove(MP_PlayerState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AMP_PlayerState* AttackerState = Attacker->GetPlayerState<AMP_PlayerState>();
	AMP_PlayerState* VictimState = Victim->GetPlayerState<AMP_PlayerState>();
	if (AttackerState == nullptr || VictimState == nullptr) return BaseDamage;
	if (VictimState == AttackerState) // 自己伤害自己
	{
		return BaseDamage;
	}
	if (AttackerState->GetTeam() == VictimState->GetTeam()) // 不同队伍
	{
		return 0.f;
	}

	return BaseDamage;
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// 团队对抗，实现两边人数平衡
	AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
	if (MP_GameState)
	{
		for (TObjectPtr<APlayerState> PlayerState : MP_GameState->PlayerArray)
		{
			AMP_PlayerState* MP_PlayerState = Cast<AMP_PlayerState>(PlayerState.Get());
			if (MP_PlayerState && MP_PlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (MP_GameState->BlueTeam.Num() >= MP_GameState->RedTeam.Num())
				{
					MP_GameState->RedTeam.AddUnique(MP_PlayerState);
					MP_PlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else if (MP_GameState->BlueTeam.Num() < MP_GameState->RedTeam.Num())
				{
					MP_GameState->BlueTeam.AddUnique(MP_PlayerState);
					MP_PlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
