// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* Instigatorpawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Instigatorpawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (Instigatorpawn->HasAuthority())	// Server
			{
				if (Instigatorpawn->IsLocallyControlled()) // Server host, Use replicates projectile, no SSR
				{
					SpawnedProjectile = 
					World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
				else   // Server, not locally controlled - spawn non-replicated projectile, no SSR
				{
					SpawnedProjectile =
						World->SpawnActor<AProjectile>(
							ServerSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
			}
			else  // Client, using SSR
			{
				if (Instigatorpawn->IsLocallyControlled()) // Client, locally controlled , Spawn non-replicated projectile, use SSR
				{
					SpawnedProjectile =
						World->SpawnActor<AProjectile>(
							ServerSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;

				}
				else // Client, not locally controlled, Spawn non-replicated projectile, no SSR
				{
					SpawnedProjectile =
						World->SpawnActor<AProjectile>(
							ServerSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // Weapon not using SSR
		{
			if (Instigatorpawn->HasAuthority())
			{
				SpawnedProjectile =
					World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
		}
	}
}
