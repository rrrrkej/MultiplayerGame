// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_AnimInstance.h"
#include "MP_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerTPS/DebugHeader.h"
#include "MultiplayerTPS/Weapon/Weapon.h"
#include "MultiplayerTPS/Types/CombatState.h"

void UMP_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MP_Character = Cast<AMP_Character>(TryGetPawnOwner());

}

void UMP_AnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MP_Character == nullptr)
	{
		MP_Character = Cast<AMP_Character>(TryGetPawnOwner());
	}

	if (MP_Character == nullptr) return;

	// Get Character velocity
	FVector Velocity = MP_Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	// Get Character if in air
	bIsInAir = MP_Character->GetCharacterMovement()->IsFalling();

	// Get Character if Accelerating
	bIsAccelerating = MP_Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	
	// Update bWeaponEquipped state
	bWeaponEquipped = MP_Character->IsWeaponEquipped();
	EquippedWeapon = MP_Character->GetEquippedWeapon();

	//Update bIsCrouch state
	bIsCrouched = MP_Character->bIsCrouched;

	// Update bAiming state
	bAiming = MP_Character->IsAiming();

	//Update TurningInPlace
	TurningInPlace = MP_Character->GetTurningInPlace();
	bRotateRootBone = MP_Character->ShouldRotateRootBone();

	// Update YawOffset
	FRotator AimRotation = MP_Character->GetBaseAimRotation();//this founction return the world rotation of controller
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MP_Character->GetVelocity());//this founction return the world rotation of argument
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 10.f);
	YawOffset = DeltaRotation.Yaw;

	// Update Lean
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MP_Character->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	// Update AimOffset
	AO_Yaw = MP_Character->GetAO_Yaw();
	AO_Pitch = MP_Character->GetAO_Pitch();

	// Update Elim
	bElimmed = MP_Character->IsElimmed();
		
	// Update RightHand Transform and LeftHandTransform
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MP_Character->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MP_Character->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (MP_Character->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = 
				UKismetMathLibrary::FindLookAtRotation(
				RightHandTransform.GetLocation(),
				RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MP_Character->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	} 

	/**
	* Update boolean in AnimtionBlueprint
	* 1.ECombatState:Character state, declare in CombatComponent.h
	* 2.bDisableGameplay:alter when GameState change
	*/
	bUseFABRIK = MP_Character->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = MP_Character->GetCombatState() != ECombatState::ECS_Reloading && !MP_Character->GetDisableGameplay();
	bTransformRightHand = MP_Character->GetCombatState() != ECombatState::ECS_Reloading && !MP_Character->GetDisableGameplay();
}
