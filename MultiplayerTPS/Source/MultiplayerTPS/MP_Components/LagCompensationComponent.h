// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;	// key:name of BoxComponent, value: related info
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfimed;

	UPROPERTY()
	bool bHeadShot;
};

class AMP_PlayerController;
class AMP_Character;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIPLAYERTPS_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AMP_Character;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, FColor Color = FColor::Orange);
	
	// Handle server-side rewind(Only work in server, Not RPC)
	FServerSideRewindResult ServerSideRewind(AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	// Store FFramePackage for MaxRecordTime
	TDoubleLinkedList<FFramePackage> FrameHistory;

	// Called by client, hit request
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		AWeapon* Weapon
	);

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	//	Calculate interpolation framepackage
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	// Handle Server-Side Rewind hit infomation
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);
	
	// Save current framepackage
	void CacheBoxPositions(AMP_Character* HitCharacter, FFramePackage& OutFramepackage);
	// Move current framepackage to HitTime framepackage
	void MoveBoxes(AMP_Character* HitCharacter, const FFramePackage& Package);
	// Move back current framepackage
	void ResetHitBoxes(AMP_Character* HitCharacter, const FFramePackage& Package);

	void EnableCharacterMeshCollision(AMP_Character* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

private:
	UPROPERTY()
	AMP_Character* Character;
	
	UPROPERTY()
	AMP_PlayerController* Controller;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
	// Record FramePackage process, insert at head node
	void RecordFrame();
};
