// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_PlayerController.h"
#include "MultiplayerTPS/UserWidget/MP_HUD.h"
#include "MultiplayerTPS/UserWidget/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AMP_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	MP_HUD = Cast<AMP_HUD>(GetHUD());

}

void AMP_PlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MP_HUD = MP_HUD == nullptr ? Cast<AMP_HUD>(GetHUD()) : MP_HUD;

	if (MP_HUD && 
		MP_HUD->CharacterOverlay && 
		MP_HUD->CharacterOverlay->HealthBar && 
		MP_HUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MP_HUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MP_HUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}