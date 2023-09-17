// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MP_HUD.generated.h"

class UTexture2D;

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

private:
	FCrosshairPackage CrosshairPackage;

public:
	FORCEINLINE void SetCrosshairPackage(const FCrosshairPackage& Package) { CrosshairPackage = Package; }
};
