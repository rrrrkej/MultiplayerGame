// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetWork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"

#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/Character/MP_Character.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	//construct SkeletalMeshComponent
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	//construct SphereComponent
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//construct PickupWidget
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
	PickupWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}



// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

// Bind delegate with AreaSphere
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMP_Character* MP_Character = Cast<AMP_Character>(OtherActor);
	if (MP_Character)
	{
		MP_Character-> SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMP_Character* MP_Character = Cast<AMP_Character>(OtherActor);
	if (MP_Character)
	{
		MP_Character->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowScopeWidget_Implementation(bool bIsAiming)
{
	/*if (!ScopeWidget && ScopeWidgetClass)
	{
		ScopeWidget = CreateWidget<UUserWidget>(OwnerPlayerController, ScopeWidgetClass);
		ScopeZoomIn = ScopeWidget->GetAnimationByName(TEXT("ScopeZoomIn"));
		ScopeWidget->AddToViewport();
	}
	if (ScopeZoomIn)
		DebugHeader::Print("ScopeZoomIn true", FColor::Cyan);
	if (ScopeWidget)
	{
		if (bIsAiming)
		{
			ScopeWidget->PlayAnimation(ScopeZoomIn, 0.f, 1, EUMGSequencePlayMode::Forward);
			DebugHeader::Print("ShowScopeWidget true", FColor::Green);
			ScopeWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ScopeWidget->PlayAnimation(ScopeZoomIn, 0.f, 1, EUMGSequencePlayMode::Reverse);
			DebugHeader::Print("ShowScopeWidget false", FColor::Red);
			ScopeWidget->SetVisibility(ESlateVisibility::Hidden);
		}	
	}*/
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		HandleStateInEquipped();
		break;
	case EWeaponState::EWS_Dropped:
		HandleStateInDropped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		HandleStateInEquippedSecondary();
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		HandleStateInEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		HandleStateInEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		HandleStateInDropped();
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::HandleStateInEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	如果武器有物理模拟效果（比如SMG的带子），则需要开启物理效果
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	//	关闭轮廓光
	EnableCustomDepth(false);
}

void AWeapon::HandleStateInEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	如果武器有物理模拟效果（比如SMG的带子），则需要开启物理效果
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	//	关闭轮廓光
	EnableCustomDepth(false);
	//	GetWeaponMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	//	GetWeaponMesh()->MarkRenderStateDirty();
}

void AWeapon::HandleStateInDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//	打开轮廓光
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMP_Character>(GetOwner()) : OwnerCharacter;
	
	//逐壳装填时，装满停止。
	if (OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsMagFull() && this->WeaponType == EWeaponType::EWT_Shotgun)
	{
		OwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}    
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerPlayerController = nullptr;
	}
	else
	{
		OwnerCharacter = Cast<AMP_Character>(Owner);
		if (OwnerCharacter && OwnerCharacter->GetEquippedWeapon() && OwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
		
	}
}

void AWeapon::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMP_Character>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AMP_PlayerController>(OwnerCharacter->Controller) : OwnerPlayerController;
		if (OwnerPlayerController)
		{
			OwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsMagFull()
{
	return Ammo == MagCapacity;
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget); 
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			if (GetWorld())
			{
				GetWorld()->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}		  
	}
	if (HasAuthority())
	{
		SpendRound();
	}
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerPlayerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();


	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc / ToEndLoc.Size() * TRACE_LENGTH);
}