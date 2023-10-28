// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "Kismet/GamePlayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

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
	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);		// 计算命中结果

		AMP_Character* MP_Character = Cast<AMP_Character>(FireHit.GetActor());
		if (MP_Character)
		{
			if (HitMap.Contains(MP_Character))
			{
				HitMap[MP_Character]++;
			}
			else
			{
				HitMap.Emplace(MP_Character, 1);
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
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,	//	Character be hitted
					Damage * HitPair.Value,	//	Multiplay Damage by number of times hit
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
		//DrawDebugSphere(GetWorld(), FireHit.ImpactPoint, 10, 16, FColor::Orange, true);
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
