// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_HUD.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"

void AMP_HUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void AMP_HUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();

	}
}

void AMP_HUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * CrosshairPackage.CrosshairSpread;

		if (CrosshairPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(CrosshairPackage.CrosshairCenter, ViewportCenter, Spread, CrosshairPackage.CrosshairColor);
		}
		if (CrosshairPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(CrosshairPackage.CrosshairLeft, ViewportCenter, Spread, CrosshairPackage.CrosshairColor);
		}
		if (CrosshairPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(CrosshairPackage.CrosshairRight, ViewportCenter, Spread, CrosshairPackage.CrosshairColor);
		}
		if (CrosshairPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(CrosshairPackage.CrosshairTop, ViewportCenter, Spread, CrosshairPackage.CrosshairColor);
		}
		if (CrosshairPackage.CrosshairButtom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(CrosshairPackage.CrosshairButtom, ViewportCenter, Spread, CrosshairPackage.CrosshairColor);
		}
	}
}

void AMP_HUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);

}
