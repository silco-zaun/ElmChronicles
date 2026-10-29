// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "../../Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 에디터 프리뷰에서는 Pawn이 없으므로 반드시 체크한다.
	APlayerCharacter* Character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(Character))
		return;

	// State.Dead 태그를 매 프레임 읽는다. 리스폰할 때 태그만 지우면 자동으로 원래 포즈로 돌아온다.
	mIsDead = Character->IsDead();

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

	// Z속도를 빼고 수평 속도만 사용한다. (낙하 속도가 달리기로 섞이지 않도록)
	mVelocityLength = Movement->Velocity.Size2D();
	mIsRunning = mVelocityLength > 0.0;
	mIsFalling = Movement->IsFalling();
	mIsAccelerating = Movement->GetCurrentAcceleration().SizeSquared() > 0.f;
}


void UPlayerAnimInstance::PlayHit()
{
	// 재생 중이어도 다시 Play하면 처음부터 재생된다.
	if (IsValid(mHitMontage))
		Montage_Play(mHitMontage);
}

void UPlayerAnimInstance::PlaySkill1()
{

}

void UPlayerAnimInstance::PlaySkill1(const FString& SectionName)
{
}

void UPlayerAnimInstance::AnimNotify_DeathEnd()
{

}
