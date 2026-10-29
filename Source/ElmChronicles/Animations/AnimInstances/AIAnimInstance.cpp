// Fill out your copyright notice in the Description page of Project Settings.


#include "AIAnimInstance.h"
#include "../../Characters/AICharacter.h"
#include "Animation/AnimMontage.h"

UAIAnimInstance::UAIAnimInstance()
{
}

void UAIAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UAIAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UAIAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AAICharacter* AICharacter = Cast<AAICharacter>(TryGetPawnOwner());

	if (IsValid(AICharacter))
	{
		if (AICharacter->IsDead())
		{
			mAnimType = EMonsterAnimType::Death;
		}
		else if (AICharacter->GetVelocity().Size2D() > 10.f)
		{
			mAnimType = EMonsterAnimType::Run;
		}
		else
		{
			mAnimType = EMonsterAnimType::Idle;
		}
	}
}

void UAIAnimInstance::AnimNotify_DeathEnd()
{
	AAICharacter* AICharacter = Cast<AAICharacter>(TryGetPawnOwner());

	if (IsValid(AICharacter))
		AICharacter->Death();
}

void UAIAnimInstance::PlayHit()
{
	// 재생 중이어도 다시 Play하면 처음부터 재생된다.
	if (IsValid(mHitMontage))
		Montage_Play(mHitMontage);
}
