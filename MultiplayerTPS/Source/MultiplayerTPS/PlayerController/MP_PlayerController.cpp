// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_PlayerController.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/VerticalBox.h"

#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/UserWidget/MP_HUD.h"
#include "MultiplayerTPS/UserWidget/CharacterOverlay.h"
#include "MultiplayerTPS/GameMode/MP_GameMode.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/UserWidget/Announcement.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"
#include "MultiplayerTPS/GameState/MP_GameState.h"
#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"
#include "MultiplayerTPS/Weapon/Weapon.h"
#include "MultiplayerTPS/UserWidget/ReturnToMainMenu.h"
#include "MultiplayerTPS/GameMode/TeamsGameMode.h"
#include "MultiplayerTPS/Types/Announcement.h"

void AMP_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	MP_HUD = Cast<AMP_HUD>(GetHUD());

	ServerCheckMatchState();

}

//	call at BeginPlay()
void AMP_PlayerController::ServerCheckMatchState_Implementation()
{
	AMP_GameMode* GameMode = Cast<AMP_GameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		MatchState = GameMode->GetMatchState();
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		if (MP_HUD && MatchState == MatchState::WaitingToStart)
		{
			MP_HUD->AddAnnouncement();
		}
		if (IsLocalController()) return;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AMP_PlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match,float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState); //	in case this founction happends after OnRep_MatchState has happended
	if (MP_HUD && MatchState == MatchState::WaitingToStart)
	{
		MP_HUD->AddAnnouncement();
	}
}

void AMP_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMP_PlayerController, MatchState);
	DOREPLIFETIME(AMP_PlayerController, bShowTeamScores);
	DOREPLIFETIME(AMP_PlayerController, MaxScore);
}

void AMP_PlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);

	PollInit();

	CheckPing(DeltaTime);
	
}

void AMP_PlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

// Called on server
void AMP_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMP_Character* MP_Character = Cast<AMP_Character>(InPawn);
	if (MP_Character)
	{
		// Character重生的时候，Client本地Character的Controller已经设置好了，但是Server端没设置好
		SetHUDHealth(MP_Character->GetHealth(), MP_Character->GetMaxHealth());
		SetHUDShield(MP_Character->GetShield(), MP_Character->GetMaxShield());
		SetHUDGrenades(MP_Character->GetCombatComponent()->GetGrenades()); 
		if (MP_Character->GetCombatComponent() && MP_Character->GetCombatComponent()->GetEquippedWeapon())
		{
			SetHUDWeaponAmmo(MP_Character->GetCombatComponent()->GetEquippedWeapon()->GetAmmo());
			SetHUDCarriedAmmo(MP_Character->GetCombatComponent()->GetCarriedAmmo());
		}
	}
}

#pragma region SetHUD
void AMP_PlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->HealthBar &&
		MP_HUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		MP_HUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MP_HUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMP_PlayerController::SetHUDShield(float Shield, float MaxShidle)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->ShieldBar &&
		MP_HUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShidle;
		MP_HUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShidle));
		MP_HUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShidle;
	}
}

void AMP_PlayerController::SetHUDScore(float Score)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MP_HUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AMP_PlayerController::SetHUDDefeats(int32 Defeats)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		MP_HUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AMP_PlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MP_HUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AMP_PlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MP_HUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AMP_PlayerController::SetHUDGrenades(int32 GrenadesNum)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), GrenadesNum);
		MP_HUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = GrenadesNum;
	}
}

void AMP_PlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		//	Handle limb condition scuh as at begining
		if (CountdownTime < 0.f)
		{
			MP_HUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MP_HUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AMP_PlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->Announcement &&
		MP_HUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		//	Handle limb condition scuh as at begining
		if (CountdownTime < 0.f)
		{
			MP_HUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MP_HUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AMP_PlayerController::HideTeamScores()
{
	//DebugHeader::Print("HideTeamScores()", FColor::Red);
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->TeamScoreBox;
	
	// 隐藏界面
	if (bHUDValid)
	{
		MP_HUD->CharacterOverlay->TeamScoreBox->SetRenderOpacity(0.f);
	}
}

void AMP_PlayerController::InitTeamScores()
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD && MP_HUD->CharacterOverlay;

	if (bHUDValid && MP_HUD->CharacterOverlay->LeftScoreText)
	{
		MP_HUD->CharacterOverlay->LeftScoreText->SetText(FText::FromString("0"));
	}
	if (bHUDValid && MP_HUD->CharacterOverlay->RightScoreText)
	{
		MP_HUD->CharacterOverlay->RightScoreText->SetText(FText::FromString("0"));
	}
	if (bHUDValid && MP_HUD->CharacterOverlay->LeftScoreProgressBar)
	{
		MP_HUD->CharacterOverlay->LeftScoreProgressBar->SetPercent(0.f);
	}
	if (bHUDValid && MP_HUD->CharacterOverlay->RightScoreProgressBar)
	{
		MP_HUD->CharacterOverlay->RightScoreProgressBar->SetPercent(0.f);
	}
	// 显示界面
	if (bHUDValid && MP_HUD->CharacterOverlay->TeamScoreBox)
	{
		MP_HUD->CharacterOverlay->TeamScoreBox->SetRenderOpacity(1.f);
	}
}

void AMP_PlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->RightScoreText &&
		MP_HUD->CharacterOverlay->RightScoreProgressBar;

	if (bHUDValid)
	{
		FString Score = FString::Printf(TEXT("%d"), RedScore);
		float Percentage = (float)RedScore  / MaxScore;
		MP_HUD->CharacterOverlay->RightScoreText->SetText(FText::FromString(Score));
		MP_HUD->CharacterOverlay->RightScoreProgressBar->SetPercent(Percentage);
	}
}

void AMP_PlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD && 
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->LeftScoreText &&
		MP_HUD->CharacterOverlay->LeftScoreProgressBar;

	if (bHUDValid)
	{
		FString Score = FString::Printf(TEXT("%d"), BlueScore);
		float Percentage = (float)BlueScore / MaxScore;
		MP_HUD->CharacterOverlay->LeftScoreText->SetText(FText::FromString(Score));
		MP_HUD->CharacterOverlay->LeftScoreProgressBar->SetPercent(Percentage);
	}
}

void AMP_PlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		MP_GameMode = MP_GameMode == nullptr ? Cast<AMP_GameMode>(UGameplayStatics::GetGameMode(this)) : MP_GameMode; 
		if (MP_GameMode)
		{
			SecondsLeft = FMath::CeilToInt(MP_GameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void AMP_PlayerController::HighPingWarning()
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay->Wifi_img &&
		MP_HUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		MP_HUD->CharacterOverlay->Wifi_img->SetOpacity(1.f);
		MP_HUD->CharacterOverlay->PlayAnimation(
			MP_HUD->CharacterOverlay->HighPingAnimation,
			0.f,
			3
		);
	}
}

void AMP_PlayerController::StopHighPingWarning()
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	bool bHUDValid = MP_HUD &&
		MP_HUD->CharacterOverlay->Wifi_img &&
		MP_HUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		MP_HUD->CharacterOverlay->Wifi_img->SetOpacity(0.f);
		if (MP_HUD->CharacterOverlay->IsAnimationPlaying(MP_HUD->CharacterOverlay->HighPingAnimation))
		{
			MP_HUD->CharacterOverlay->StopAnimation(MP_HUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AMP_PlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (MP_HUD && MP_HUD->CharacterOverlay)
		{
			CharacterOverlay = MP_HUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth)
				{
					bInitializeHealth = false;
					SetHUDHealth(HUDHealth, HUDMaxHealth);
				}
				if (bInitializeShield)
				{
					bInitializeShield = false;
					SetHUDShield(HUDShield, HUDMaxShield);
				}
				if (bInitializeScore)
				{
					bInitializeScore = false;
					SetHUDScore(HUDScore);
				}
				if (bInitializeDefeats)
				{
					bInitializeDefeats = false;
					SetHUDDefeats(HUDDefeats);
				}
				if (bInitializeCarriedAmmo)
				{
					bInitializeCarriedAmmo = false;
					SetHUDCarriedAmmo(HUDCarriedAmmo);
				}
				if (bInitializeWeaponAmmo)
				{
					bInitializeWeaponAmmo = false;
					SetHUDWeaponAmmo(HUDWeaponAmmo);
				}
				if (bInitializeGrenades)
				{
					bInitializeGrenades = false;
					SetHUDGrenades(HUDGrenades);
				}
				ShowTeamScores(bShowTeamScores);
			}
		}
	}
}

#pragma endregion
void AMP_PlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AMP_PlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientRePortServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMP_PlayerController::ClientRePortServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AMP_PlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else    return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMP_PlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
	/*	PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (!HasAuthority() && GetPawn()->IsLocallyControlled())
		{
			DebugHeader::Print(FString::Printf(TEXT("Get Ping(): %f"), PlayerState->GetPingInMilliseconds()), FColor::Blue);
		}*/
		
		if (PlayerState)
		{
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPintStatus(true);
			}
			else
			{
				ServerReportPintStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	if (MP_HUD &&
		MP_HUD->CharacterOverlay &&
		MP_HUD->CharacterOverlay->HighPingAnimation &&
		MP_HUD->CharacterOverlay->IsAnimationPlaying(MP_HUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AMP_PlayerController::ServerReportPintStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AMP_PlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMP_PlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMP_PlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;	// Replicates 变量

	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	if (MP_HUD)
	{
		if(MP_HUD->CharacterOverlay == nullptr)
			MP_HUD->AddCharacterOverlay();

		if (MP_HUD->Announcement)
		{
			MP_HUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		// Teamsmode widget visibility
		if (!HasAuthority()) return; 
		ShowTeamScores(bShowTeamScores);
	}
}

void AMP_PlayerController::HandleCooldown()
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	//DebugHeader::Print("HandleCooldown()");
	if (MP_HUD)
	{
		MP_HUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = MP_HUD->Announcement
			&& MP_HUD->Announcement->AnnouncementText
			&& MP_HUD->Announcement->InfoText;

		if (bHUDValid)
		{
			//DebugHeader::Print("bHUDValid true");
			MP_HUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			MP_HUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			//	Set InfoText
			AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
			AMP_PlayerState* MP_PlayerState = GetPlayerState<AMP_PlayerState>();
			if (MP_GameState && MP_PlayerState)
			{
				TArray<AMP_PlayerState*> TopPlayers = MP_GameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamInfoText(MP_GameState): GetInfoText(TopPlayers);

				MP_HUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}

	AMP_Character* MP_Character = Cast<AMP_Character>(GetPawn());
	if (MP_Character)
	{
		MP_Character->bDisableGameplay = true;
		if (MP_Character->GetCombatComponent())
		{
			MP_Character->GetCombatComponent()->FireButtonpressed(false);
		}
	}
}

FString AMP_PlayerController::GetInfoText(const TArray<AMP_PlayerState*>& Players)
{
	AMP_PlayerState* MP_PlayerState = GetPlayerState<AMP_PlayerState>(); // 本地玩家标记
	if (!MP_PlayerState) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == MP_PlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiesForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString AMP_PlayerController::GetTeamInfoText(AMP_GameState* MP_GameState)
{
	if (!MP_GameState) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = MP_GameState->RedTeamScore;
	const int32 BlueTeamScore = MP_GameState->BlueTeamScore;
	
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}
	return InfoTextString;
}

void AMP_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(PlayerControllerMapping, 0);
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ReturnMenuAction, ETriggerEvent::Triggered, this, &AMP_PlayerController::ReturnMenu_Pressed);
	}
}

void AMP_PlayerController::ReturnMenu_Pressed(const FInputActionValue& Value)
{
	if (ReturnToMainMenuClass == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AMP_PlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AMP_PlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
		if (MP_HUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				MP_HUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				MP_HUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				MP_HUD->AddElimAnnouncement("You", "Yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				MP_HUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			else
			{
				MP_HUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
			}
		}
	}
}


void AMP_PlayerController::OnRep_ShowTeamScores()
{
	ShowTeamScores(bShowTeamScores);
}

void AMP_PlayerController::ShowTeamScores(bool bShow)
{
	if (bShow)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}