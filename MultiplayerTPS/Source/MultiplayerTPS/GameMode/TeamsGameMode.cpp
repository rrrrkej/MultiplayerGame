// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "MultiplayerTPS/GameState/MP_GameState.h"
#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMode = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 当前比赛模式下，PlayerController里有需要用的参数
	AMP_PlayerController* PlayerController = Cast<AMP_PlayerController>(NewPlayer);
	PlayerController->SetMaxScore(MaxScore);
	PlayerController->bShowTeamScores = true;

	// 团队对抗，实现两边人数平衡
	AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
	if (MP_GameState)
	{
		MP_GameState->SetMaxScore(MaxScore);
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

void ATeamsGameMode::PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
	
	// 攻击者所属队伍加分
	AMP_PlayerState* AttackerPlayerState = AttackerController ? Cast<AMP_PlayerState>(AttackerController->PlayerState) : nullptr;
	if (MP_GameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			MP_GameState->BlueTeamScores();
		}
		else if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			MP_GameState->RedTeamScores();
		}
	}
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

