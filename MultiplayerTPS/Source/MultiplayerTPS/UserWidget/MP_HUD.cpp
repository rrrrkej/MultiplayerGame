// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_HUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"

#include "ElimAnnouncement.h"
#include "MultiplayerTPS/DebugHeader.h"

void AMP_HUD::BeginPlay()
{
	Super::BeginPlay();

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

void AMP_HUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();

	}
}

void AMP_HUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			for (UElimAnnouncement* ElimWidget : ElimWidgets)
			{
				if (ElimWidget && ElimWidget->AnnouncementBox)
				{
					// 获得AnnoucnementBox在CanvasPannel上的位置等属性
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ElimWidget->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			ElimWidgets.Add(ElimAnnouncementWidget);
		}

		FTimerHandle ElimWidgetTimer;
		FTimerDelegate ElimWidgetDelegate;
		ElimWidgetDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
		// SetTimer不能接受给绑定的函数传入参数，所以这里另外绑定委托函数并传入参数，再设置Timer控制触发时间
		GetWorldTimerManager().SetTimer(
			ElimWidgetTimer,
			ElimWidgetDelegate,
			ElimAnnouncementTime,
			false
		);
	}
}

void AMP_HUD::ElimAnnouncementTimerFinished(UElimAnnouncement* WidgetToRemove)
{
	if (WidgetToRemove)
	{
		WidgetToRemove->RemoveFromParent();
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
