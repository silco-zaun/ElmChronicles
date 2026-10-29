// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../MonsterAnimTypes.h"
#include "Animation/AnimInstance.h"
#include "AIAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UAIAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UAIAnimInstance();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMonsterAnimType mAnimType = EMonsterAnimType::Idle;

	// 피격 몽타주 (Additive). 몬스터별로 자식 ABP의 Class Defaults에서 지정한다.
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> mHitMontage;

public:
	virtual void NativeInitializeAnimation();
	virtual void NativeBeginPlay();
	virtual void NativeUpdateAnimation(float DeltaSeconds);

public: // Notify
	// (공격 타이밍은 Send Gameplay Event 노티파이가 어빌리티에게 직접 보낸다)
	UFUNCTION()	void AnimNotify_DeathEnd();

public:
	void PlayHit();
};
