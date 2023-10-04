// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_PlayerController.h"
#include "MultiplayerTPS/UserWidget/MP_HUD.h"
#include "MultiplayerTPS/UserWidget/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MultiplayerTPS/Character/MP_Character.h"

void AMP_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	MP_HUD = Cast<AMP_HUD>(GetHUD());

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


