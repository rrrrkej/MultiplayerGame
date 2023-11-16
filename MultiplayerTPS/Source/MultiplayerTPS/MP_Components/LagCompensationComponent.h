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

	UPROPERTY()
	AMP_Character* Character;	//	作为存储包时，此处为本机charcter，也就是初始值；作为GetFrameToCheck()返回值时，返回的是HitCharacter及其信息
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

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AMP_Character*, uint32> HeadShots;
	UPROPERTY()
	TMap<AMP_Character*, uint32> BodyShots;
};

class AMP_PlayerController;
class AMP_Character;
class AWeapon;
class AMP_GameMode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIPLAYERTPS_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AMP_Character;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, FColor Color = FColor::Orange);
	
	//SSR for Hitscan 
	FServerSideRewindResult ServerSideRewind(
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	// SSR for Projectile
	FServerSideRewindResult ProjectileServerSideRewind(
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	// SSR for Shotgun
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AMP_Character*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocation,
		float HitTime);

	// Store FFramePackage for MaxRecordTime
	TDoubleLinkedList<FFramePackage> FrameHistory;

	// ServerRPC :
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocations,
		float HitTime
	);
	
	//ServerRPC :
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	////ServerRPC :
	//UFUNCTION(Server, Reliable)
	//void ProjectileExplosiveServerScoreRequest(
	//	TArray<AMP_Character*>& HitCharacters,
	//	const FVector_NetQuantize& TraceStart,
	//	const FVector_NetQuantize100& InitialVelocity,
	//	float HitTime
	//);

	// ServerRPC :
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AMP_Character*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	//	return the package related to HitCharacter
	FFramePackage GetFrameToCheck(AMP_Character* HitCharacter, float HitTime);

	//	Calculate interpolation framepackage
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	// Handle Server-Side Rewind hit infomation
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);
	
	// Confirm hit for Projectile
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AMP_Character* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
	
	// Handle Server-Side Rewind hit infomation for shotgun
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
		);

	// Save current framepackage
	void CacheBoxPositions(AMP_Character* HitCharacter, FFramePackage& OutFramepackage);
	// Move current framepackage to HitTime framepackage
	void MoveBoxes(AMP_Character* HitCharacter, const FFramePackage& Package);
	// Move back current framepackage
	void ResetHitBoxes(AMP_Character* HitCharacter, const FFramePackage& Package);

	void EnableCharacterMeshCollision(AMP_Character* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	// Make fine adjustments to the hit time based on network quality.
	void LagTimeOffset(float& time);
private:
	UPROPERTY()
	AMP_Character* Character;
	
	UPROPERTY()
	AMP_PlayerController* Controller;

	UPROPERTY()
	AMP_GameMode* MP_GameMode;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
	// Record FramePackage process, insert at head node
	void RecordFrame();


};
