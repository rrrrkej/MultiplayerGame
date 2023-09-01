// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_AnimInstance.h"
#include "MP_Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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

}
