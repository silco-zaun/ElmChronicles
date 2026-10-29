// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "Abilities/GameplayAbility.h"
#include "ElmGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UElmGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UElmGameplayAbility();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown") float mCooldownTime = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown") FGameplayTag mCooldownTag;
	UPROPERTY(EditDefaultsOnly, Category = "Cost") float mMPCost = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Cost") float mHPCost = 0.f;
	// true면 마법 공격력 / 마법 방어력으로 계산한다.
	UPROPERTY(EditDefaultsOnly, Category = "Damage") bool mIsMagicDamage = false;
	// 몽타주 기본 재생 속도. 실제 속도 = mMontagePlayRate × AttackSpeed
	UPROPERTY(EditDefaultsOnly, Category = "Animation")	float mMontagePlayRate = 1.f;

	// mutable은 const 함수 안에서도 이 변수만은 수정할 수 있게 허용한다.
	mutable FGameplayTagContainer mCooldownTagsContainer;

public:
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// 데미지 GE 스펙을 만든다. 마법 공격이면 Damage.Magical 태그를 붙인다.
	// 공격력은 이 순간의 값으로 고정된다(Snapshot). 투사체에 실어 보낼 때도 쓴다.
	FGameplayEffectSpecHandle MakeDamageSpec() const;
	// 맞은 대상에게 데미지 GE를 적용한다. 수치는 UElmDamageExecution이 계산한다.
	void ApplyDamage(const FHitResult& Hit);
	// 아바타 앞쪽으로 구를 쓸어서(Sweep) 맞은 캐릭터 중 적대 대상에게만 ApplyDamage를 한다.
	void ApplyMeleeDamage();
	// 공격 몽타주 재생 속도. AttackSpeed가 0 이하(스탯 로드 실패)면 배율 1로 본다.
	float GetAttackPlayRate() const;
};
