// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "Kismet/GamePlayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"

#include "DrawDebugHelpers.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//	Get InstigatorController used in ApplyDamage
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.2f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			//	Line trace scan
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			FVector BeamEnd = End;
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				//	Apply Damage
				AMP_Character* MP_Character = Cast<AMP_Character>(FireHit.GetActor());
				if (MP_Character && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
						MP_Character,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}

				//	Spawn impact ParticleSystem
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				//Spawn HitTarget soundcue
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint
					);
				}
			}

			// Spawn bullet trail
			if (BeamParticle)
			{
				UParticleSystemComponent* Beam =
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						BeamParticle,
						SocketTransform
					);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}

		// Spawn MuzzleFlash
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlash,
				SocketTransform
			);
		}
		// Play Fire soundcue
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec ;
	FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Cyan, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc / ToEndLoc.Size() * TRACE_LENGTH),
		FColor::Red,
		true
	);
	return FVector(TraceStart + ToEndLoc / ToEndLoc.Size() * TRACE_LENGTH);
}
