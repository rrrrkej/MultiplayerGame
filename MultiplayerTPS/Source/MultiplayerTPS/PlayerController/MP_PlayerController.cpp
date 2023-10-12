// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_PlayerController.h"
#include "MultiplayerTPS/UserWidget/MP_HUD.h"
#include "MultiplayerTPS/UserWidget/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerTPS/GameMode/MP_GameMode.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/UserWidget/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"

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
}

void AMP_PlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);

	PollInit();
}

void AMP_PlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMP_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMP_Character* MP_Character = Cast<AMP_Character>(InPawn);
	if (MP_Character)
	{
		SetHUDHealth(MP_Character->GetHealth(), MP_Character->GetMaxHealth());
	}
}

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
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
		bInitializeCharacterOverlay = true;
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
		bInitializeCharacterOverlay = true;
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

void AMP_PlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	//	这边的处理看不懂
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

void AMP_PlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
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
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
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
	float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AMP_PlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else    return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMP_PlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
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

void AMP_PlayerController::HandleMatchHasStarted()
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	if (MP_HUD)
	{
		MP_HUD->AddCharacterOverlay();
		if (MP_HUD->Announcement)
		{
			MP_HUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMP_PlayerController::HandleCooldown()
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;
	if (MP_HUD)
	{
		MP_HUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = MP_HUD->Announcement
			&& MP_HUD->Announcement->AnnouncementText
			&& MP_HUD->Announcement->InfoText;

		if (bHUDValid)
		{
			MP_HUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match States In:");
			MP_HUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			MP_HUD->Announcement->InfoText->SetVisibility(ESlateVisibility::Hidden);
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