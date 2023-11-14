// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerTPS/Types/Team.h"
#include "TeamPlayerStart.generated.h"

class UMaterialInstance;
class UMaterialInstanceDynamic;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
	ATeamPlayerStart();

	virtual void BeginPlay() override;
public:
	UPROPERTY(EditAnywhere)
	ETeam Team = ETeam::ET_NoTeam;

protected:
	// 
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PlayerStartMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* SpawnPadMaterial;

	UMaterialInstanceDynamic* SpawnPadMaterialDynamic;
};
