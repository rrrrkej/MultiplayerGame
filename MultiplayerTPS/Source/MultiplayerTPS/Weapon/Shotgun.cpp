// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GamePlayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/MP_Components/LagCompensationComponent.h"
#include "MultiplayerTPS/DebugHeader.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	//	Play FireAnim and update ammo
	AWeapon::Fire(FVector());

	//	Get InstigatorController used in ApplyDamage
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) return;
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();

	// Maps hit chracter to number of times hit
	TMap<AMP_Character*, uint32> HitMap;
	TMap<AMP_Character*, uint32> HeadShotHitMap;
	for (const FVector_NetQuantize& HitTarget : HitTargets)
	{
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);		// 计算命中结果

		AMP_Character* MP_Character = Cast<AMP_Character>(FireHit.GetActor());
		if (MP_Character)
		{
			const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

			if (bHeadShot)
			{
				if (HeadShotHitMap.Contains(MP_Character))	HeadShotHitMap[MP_Character]++;
				else   HeadShotHitMap.Emplace(MP_Character, 1);
			}
			else
			{
				if (HitMap.Contains(MP_Character))	HitMap[MP_Character]++;
				else   HitMap.Emplace(MP_Character, 1);
			}
		
			//	Spawn impact ParticleSystem
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			//Spawn HitTarget soundcue if hit character
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
		}
	}
	TArray<AMP_Character*> HitCharacters;	// Array to HitCharacters
	TMap<AMP_Character*, float> DamageMap;	// Maps Character to total damage
	// Body hit damage
	for (auto HitPair : HitMap)
	{
		if (HitPair.Key)
		{
			DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
			HitCharacters.AddUnique(HitPair.Key);
		}
	}
	// Head hit damage
	for (auto HitPair : HeadShotHitMap)
	{
		if (HitPair.Key)
		{
			if (DamageMap.Contains(HitPair.Key)) DamageMap[HitPair.Key] += HeadShotDamage * HitPair.Value;
			else DamageMap.Emplace(HitPair.Key, HitPair.Value * HeadShotDamage);
			HitCharacters.AddUnique(HitPair.Key);
		}
	}
	// Loop through DamageMap to get total damage for each character
	for (TPair<AMP_Character*, float>  DamagePair : DamageMap)
	{
		if (DamagePair.Key && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				UGameplayStatics::ApplyDamage(
					DamagePair.Key,	//	Character be hitted
					DamagePair.Value,	//	total damage
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
	

	//	Apply damage with LagCompensation
	if (!HasAuthority() && bUseServerSideRewind)
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMP_Character>(OwnerPawn) : OwnerCharacter;
		OwnerController = OwnerController == nullptr ? Cast<AMP_PlayerController>(InstigatorController) : OwnerController;

		if (OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
		{
			OwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(
				HitCharacters,
				Start,
				HitTargets,
				OwnerController->GetServerTime() - OwnerController->SingleTripTime
			);
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc / ToEndLoc.Size() * TRACE_LENGTH;
		HitTargets.Add(ToEndLoc);
	}
}
