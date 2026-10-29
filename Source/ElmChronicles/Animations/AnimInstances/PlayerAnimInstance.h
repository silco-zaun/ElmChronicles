// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class ELMCHRONICLES_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPlayerAnimInstance();

protected:
	// 수평 이동 속도 (블렌드 스페이스 입력)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double mVelocityLength = 0.0;

	// 달리는 중인지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool mIsRunning = false;

	// 공중에 떠 있는지 (점프/낙하)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool mIsFalling = false;

	// 이동 입력이 들어오고 있는지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool mIsAccelerating = false;

	// 죽었는지. ABP의 Blend Poses by bool이 이 값으로 사망 포즈를 고른다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool mIsDead = false;
	
	// 피격 몽타주 (Additive).
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> mHitMontage;

public:
	virtual void NativeInitializeAnimation();
	virtual void NativeBeginPlay();
	virtual void NativeUpdateAnimation(float DeltaSeconds);

public:
	void PlayHit();
	void PlaySkill1();
	void PlaySkill1(const FString& SectionName);

public: // Notify
	// 노티파이 함수 생성방법 : void AnimNotify_노티파이이름() 으로 함수를
	// 만든다.
	// (공격/콤보 타이밍은 Send Gameplay Event 노티파이가 어빌리티에게 직접 보낸다)
	UFUNCTION()
	void AnimNotify_DeathEnd();
};
