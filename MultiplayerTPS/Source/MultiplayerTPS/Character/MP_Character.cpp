// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Timermanager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "MultiplayerTPS/PlayerState/MP_PlayerState.h"
#include "MultiplayerTPS/Weapon/WeaponTypes.h"
#include "MultiplayerTPS/Weapon/Weapon.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"
#include "MultiplayerTPS/MP_Components/BuffComponent.h"
#include "MultiplayerTPS/Character/MP_AnimInstance.h"
#include "MultiplayerTPS/MultiplayerTPS.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/GameMode/MP_GameMode.h"
#include "MultiplayerTPS/UserWidget/OverheadWidget.h"
#include "MultiplayerTPS/MP_Components/LagCompensationComponent.h"
#include "MultiplayerTPS/GameState/MP_GameState.h"
#include "../DebugHeader.h"

AMP_Character::AMP_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize Character setting
	// 生成时的碰撞处理策略，如果检测到了碰撞，则会查找附近没有碰撞的位置进行生成
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// initialize CameraBoom attribute
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	// initialize FollowCamera attribute
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Construct overhead WidgetComponent
	OverheadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidgetComponent->SetupAttachment(RootComponent);
	OverheadWidgetComponent->SetIsReplicated(true);

	// Construct CombatComponent
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);
	
	// Construct BuffComponent
	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	// Construct LagCompensationComponent
	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	// Set properties in CMC
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);

	// Set Camera block
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	

	// Set Network properties
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// initial properties
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// Construct TimelineComponent
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// Construct GrenadeComponent
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

#pragma region HitboxBinding
	/**
	* Hit boxes for server-side rewind, parameter name as same as skeleton name
	*/
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (TPair<FName, UBoxComponent*>& BoxPair : HitCollisionBoxes)
	{
		if (BoxPair.Value)
		{
			BoxPair.Value->SetCollisionObjectType(ECC_HitBox);
			BoxPair.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
#pragma endregion
}

void AMP_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 在AMP_Character的生命周期内，如果OverlappingWeapon的值发生了变化，服务器会发送到每一个客户端
	//DOREPLIFETIME(AMP_Character, OverlappingWeapon);
	// 条件发送
	DOREPLIFETIME_CONDITION(AMP_Character, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMP_Character, Health);
	DOREPLIFETIME(AMP_Character, Shield);
	DOREPLIFETIME(AMP_Character, bDisableGameplay);
	
}

void AMP_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeed(
			GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (Controller)
		{
			LagCompensationComponent->Controller = Cast<AMP_PlayerController>(Controller);
		}
	}
}

void AMP_Character::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimuProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AMP_Character::Elim(bool bPlayerLeftGame)
{
	if (CombatComponent)
	{
		if (CombatComponent->EquippedWeapon)
			HandleWeaponWhenElimed(CombatComponent->EquippedWeapon);
		if (CombatComponent->SecondaryWeapon)
			HandleWeaponWhenElimed(CombatComponent->SecondaryWeapon);
	}

	MulticastElim(bPlayerLeftGame);
}


void AMP_Character::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 100.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	if (CombatComponent)
	{
		CombatComponent->FireButtonpressed(false);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn ElimBot component
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent =
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ElimBotEffect,
				ElimBotSpawnPoint,
				GetActorRotation()
			);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	// Turn off the weapon widget
	if (IsLocallyControlled() &&
		CombatComponent &&
		CombatComponent->EquippedWeapon &&
		CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{

		CombatComponent->EquippedWeapon->ShowScopeWidget(false);
	}

	// Set timer
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AMP_Character::ElimTimerFinished,
		ElimDelay
	);

	// Destroy the component if it exists
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void AMP_Character::ElimTimerFinished()
{
	MP_GameMode = MP_GameMode == nullptr ? GetWorld()->GetAuthGameMode<AMP_GameMode>() : MP_GameMode;
	if (MP_GameMode && !bLeftGame)
	{
		MP_GameMode->RequestRespawn(this, Controller);
	}

	if (bLeftGame && IsLocallyControlled()) // if leave game ,broadcast
	{
		OnLeftGame.Broadcast();
	}

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void AMP_Character::HandleWeaponWhenElimed(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon && Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void AMP_Character::Destroyed()
{
	Super::Destroyed();
	// 
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	// Destroy the weapon when GameSate become Cooldown
	AMP_GameMode* GameMode = Cast<AMP_GameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = GameMode && GameMode->GetMatchState() != MatchState::InProgress;
	if (bMatchNotInProgress && CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

void AMP_Character::ServerLeaveGame_Implementation()
{
	MP_GameMode = MP_GameMode == nullptr ? GetWorld()->GetAuthGameMode<AMP_GameMode>() : MP_GameMode;
	MP_PlayerState = MP_PlayerState == nullptr ? GetPlayerState<AMP_PlayerState>() : MP_PlayerState;
	if (MP_GameMode && MP_PlayerState)
	{
		MP_GameMode->PlayerLeftGame(MP_PlayerState);
	}
}

void AMP_Character::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AMP_Character::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AMP_Character::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

// Called every frame
void AMP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCamera();
	PollInit();
}

void AMP_Character::PollInit()
{
	if (MP_PlayerState == nullptr)
	{	
		MP_PlayerState = GetPlayerState<AMP_PlayerState>();
		if (MP_PlayerState)
		{
			MP_PlayerState->AddToScore(0.f);
			MP_PlayerState->AddToDefeats(0);
			SetTeamColor(MP_PlayerState->GetTeam());

			AMP_GameState* MP_GameState = Cast<AMP_GameState>(UGameplayStatics::GetGameState(this));
			if (MP_GameState && MP_GameState->TopScoringPlayers.Contains(MP_PlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void AMP_Character::RotateInPlace(float DeltaTime)
{
	//	Handle
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AMP_Character::SetOverheadHealth()
{
	UOverheadWidget* OverheadWidget = Cast<UOverheadWidget>(OverheadWidgetComponent->GetUserWidgetObject());
	if (OverheadWidget)
	{
		const float HealthPercent = Health / MaxHealth;
		OverheadWidget->HealthBar->SetPercent(HealthPercent);
	}
}

void AMP_Character::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		// Spawn crown at head of character
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName("crown"),
			FVector(0.f, 0.f, 0.f),
			GetActorRotation(),
			EAttachLocation::SnapToTarget,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void AMP_Character::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void AMP_Character::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;
	switch (Team)
	{
		case ETeam::ET_NoTeam:
			GetMesh()->SetMaterial(0, OriginalMaterial);
			DissolveMaterialInstance = BlueDissolveMatInst;
			break;
		case ETeam::ET_BlueTeam:
			GetMesh()->SetMaterial(0, BlueMaterial);
			DissolveMaterialInstance = BlueDissolveMatInst;
			break;
		case ETeam::ET_RedTeam:
			GetMesh()->SetMaterial(0, RedMaterial);
			DissolveMaterialInstance = RedDissolveMatInst;
			break;
	}
}

// Called when the game starts or when spawned
void AMP_Character::BeginPlay()
{
	Super::BeginPlay();

	//	Spawn default weapon at beginning
	SpawnDefaultWeapon();
	
	// Initialzied properties in HUD
	UpdateHUDHealth();
	UpdateHUDShield();
	SetOverheadHealth();
	UpdateHUDAmmo();
	if (CombatComponent)
	{
		CombatComponent->UpdateHUDGrenades();
	}
	// Bind event in server
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMP_Character::ReceiveDamage);
	}

	if (IsLocallyControlled())
	{
		if (OverheadWidgetComponent)
		{
			OverheadWidgetComponent->SetVisibility(false);
		}
	}
	//Gamestart default settings
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	
}

void AMP_Character::AimOffset(float DeltaTime)
{
	if(CombatComponent && CombatComponent->EquippedWeapon == nullptr)  return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)	//standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(StartingAimRotation, CurrentAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;	
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)	// running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true; 
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

float AMP_Character::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMP_Character::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	SetOverheadHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AMP_Character::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void AMP_Character::UpdateHUDHealth()
{
	
	MP_PlayerController = MP_PlayerController == nullptr ? Cast<AMP_PlayerController>(Controller) : MP_PlayerController;
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AMP_Character::UpdateHUDShield()
{
	MP_PlayerController = MP_PlayerController == nullptr ? Cast<AMP_PlayerController>(Controller) : MP_PlayerController;
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AMP_Character::UpdateHUDAmmo()
{
	MP_PlayerController = MP_PlayerController == nullptr ? Cast<AMP_PlayerController>(Controller) : MP_PlayerController;
	if (MP_PlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		MP_PlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		MP_PlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
	}
}

void AMP_Character::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	//由于Pitch在网络传输的过程中由有符号数变成无符号数，所以要映射回去
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AMP_Character::SimuProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AMP_Character::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;

	//DebugHeader::Print(FString::Printf(TEXT("receive damage: %f"), Damage), FColor::Blue);
	//	Received total damage
	float TotalDamage = Damage;
	if (Shield > 0.f)
	{
		//	Calculate damage to shield
		if(Shield >= TotalDamage)
		{
			Shield = FMath::Clamp(Shield - TotalDamage, 0.f, MaxShield);
			TotalDamage = 0.f;
		}
		else
		{
			TotalDamage = FMath::Clamp(TotalDamage - Shield, 0.f, TotalDamage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - TotalDamage, 0.f, MaxHealth);

	UpdateHUDShield();
	UpdateHUDHealth();
	SetOverheadHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		MP_GameMode = MP_GameMode == nullptr ?  GetWorld()->GetAuthGameMode<AMP_GameMode>() : MP_GameMode;
		if (MP_GameMode)
		{
			MP_PlayerController = MP_PlayerController == nullptr ? Cast<AMP_PlayerController>(Controller) : MP_PlayerController;
			AMP_PlayerController* AttackerController = Cast<AMP_PlayerController>(InstigatorController);
			MP_GameMode->PlayerEliminated(this, MP_PlayerController, AttackerController);
		}
	}
}

void AMP_Character::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	 }
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AMP_Character::HideCamera()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (CombatComponent && CombatComponent->SecondaryWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}

	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (CombatComponent && CombatComponent->SecondaryWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AMP_Character::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleIronsight") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMP_Character::PlayReloadMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;

		default:
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMP_Character::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void AMP_Character::PlayHitReactMontage()
{
	// Only play when equipped
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMP_Character::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AMP_Character::PlayThrowGrenadeMontage()
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

//called by server
void AMP_Character::SetOverlappingWeapon(AWeapon* Weapon)
{
	//非严谨条件，用于出Weapon范围的时候关掉PickupWidget
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;

	//检查是否是服务器端，如果是客户端的话返回false
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AMP_Character::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//Get the variable EquippedWeapon in CombatComponent class
bool AMP_Character::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

//Get the variable bAiming in CombatComponent class
bool AMP_Character::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

AWeapon* AMP_Character::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

ECombatState AMP_Character::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return CombatComponent->CombatState;
}

FVector AMP_Character::GetHitTarget() const
{

	if (CombatComponent == nullptr) return FVector();
	return CombatComponent->HitTarget;
}

void AMP_Character::SpawnDefaultWeapon()
{
	MP_GameMode = MP_GameMode == nullptr ? Cast<AMP_GameMode>(UGameplayStatics::GetGameMode(this)) : MP_GameMode;
	UWorld* World = GetWorld();
	if (MP_GameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (CombatComponent)
		{
			CombatComponent->EquipWeapon(StartingWeapon);
		}
	}
}

bool AMP_Character::IsLocallyReloading()
{
	if (CombatComponent == nullptr) return false;

	return CombatComponent->bLocallyReloading;
}

#pragma region InputBinding

// Called to bind functionality to input
void AMP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMP_Character::JumpPressed);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMP_Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMP_Character::Look);

		// Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AMP_Character::Interaction);

		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AMP_Character::CrouchPressed);

		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AMP_Character::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AMP_Character::EndAim);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMP_Character::Fire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AMP_Character::EndFire);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AMP_Character::Reload);

		// Throw grenade
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &AMP_Character::ThrowGrenade);

		// swap to specified weapon
		EnhancedInputComponent->BindAction(PrimaryWeapon, ETriggerEvent::Triggered, this, &AMP_Character::EquipPrimaryWeapon);
		EnhancedInputComponent->BindAction(SecondaryWeapon, ETriggerEvent::Triggered, this, &AMP_Character::EquipSecondaryWeapon);
	}
}

void AMP_Character::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	// input 2D Value
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out controller forward direction
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);

		// get forward vector
		const FVector ForWardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		//add movement
		AddMovementInput(ForWardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMP_Character::Look(const FInputActionValue& Value)
{
	// input 2D Value
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void AMP_Character::JumpPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		Super::UnCrouch();
	}
	else
	{
		Super::Jump();
	}
	
}

void AMP_Character::Interaction(const FInputActionValue& Value) // typedef EquipButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComponent && CombatComponent->CombatState == ECombatState::ECS_Unoccupied)
	{
		ServerEquipButtonPressed();
		if (CombatComponent->ShouldSwapWeapons() &&
			!HasAuthority() && 
			OverlappingWeapon == nullptr)
		{
			PlaySwapMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void AMP_Character::CrouchPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		Super::UnCrouch();
	}
	else
	{
		Super::Crouch();
	}
}

void AMP_Character::Aim(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	bool bAiming = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->SetAiming(bAiming);
	}
	
}

void AMP_Character::EndAim(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	bool bAiming = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->SetAiming(bAiming);
	}
}

void AMP_Character::Fire(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	bool bFire = Value.Get<bool>();
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->FireButtonpressed(bFire);
	}
}

void AMP_Character::EndFire(const FInputActionValue& Value)
{
	bool bFire = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->FireButtonpressed(bFire);
	}
}

void AMP_Character::Reload(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	bool bReload = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void AMP_Character::ThrowGrenade(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->ThrowGrenade();
	}
}

void AMP_Character::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else if (CombatComponent->ShouldSwapWeapons())
		{
			CombatComponent->SwapWeapons();
		}
	}
}

void AMP_Character::EquipPrimaryWeapon(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->PrimaryWeaponPtr)
	{
		CombatComponent->ServerEquipSpecifiedWeapon_1();
		PlaySwapMontage();
	}
}

void AMP_Character::EquipSecondaryWeapon(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->SecondaryWeaponPtr)
	{
		CombatComponent->ServerEquipSpecifiedWeapon_2();
		PlaySwapMontage();
	}
}

#pragma endregion
