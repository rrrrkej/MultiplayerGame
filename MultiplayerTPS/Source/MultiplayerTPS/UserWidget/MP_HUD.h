// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MP_HUD.generated.h"

class UTexture2D;
class UCharacterOverlay;
class UUserWidget;
class UAnnouncement;
class APlayerController;

USTRUCT(BlueprintType)
struct FCrosshairPackage
{
	GENERATED_BODY()
public:
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairButtom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_HUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/**
	* CharacterOverlay
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	void AddCharacterOverlay();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	
	/**
	* Announcement
	*/
	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	UAnnouncement* Announcement;

	// Add Announcement userwidget to viewport
	void AddAnnouncement();
	

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	APlayerController* OwningPlayer;

	FCrosshairPackage CrosshairPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
	
	/**
	* ElimAnnouncement
	*/
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	// Elim announcement duration
	UPROPERTY()
	float ElimAnnouncementTime = 3.f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* WidgetToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimWidgets;

public:
	// Add ElimAnnouncement userwidget to viewport
	void AddElimAnnouncement(FString Attacker, FString Victim);

public:
	FORCEINLINE void SetCrosshairPackage(const FCrosshairPackage& Package) { CrosshairPackage = Package; }
};
