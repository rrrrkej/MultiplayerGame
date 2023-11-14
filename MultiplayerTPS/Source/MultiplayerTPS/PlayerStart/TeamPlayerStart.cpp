// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamPlayerStart.h"
#include "Components/StaticmeshComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Materials/MaterialInstanceDynamic.h"

ATeamPlayerStart::ATeamPlayerStart() : APlayerStart(FObjectInitializer::Get())
{
	PlayerStartMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerStartMesh"));
	PlayerStartMesh->SetupAttachment(GetRootComponent());
	PlayerStartMesh->SetRelativeLocation(FVector(0.f, 0.f, -120.f));
	GetRootComponent()->SetVisibility(true);
	PlayerStartMesh->SetVisibility(true);
}

void ATeamPlayerStart::BeginPlay()
{
	if (SpawnPadMaterial && PlayerStartMesh)
	{
		SpawnPadMaterialDynamic = UMaterialInstanceDynamic::Create(SpawnPadMaterial, this);
		PlayerStartMesh->SetMaterial(2, SpawnPadMaterial);
	}
}
