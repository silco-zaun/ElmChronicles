// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ElmGameplayAbility.h"
#include "GameplayAbility_Attack.generated.h"

// 공격 어빌리티의 공통 흐름. 플레이어 / 몬스터 / NPC가 모두 쓴다.
// 몽타주 섹션 재생 → Event.Attack.Hit마다 OnAttackHit(자식이 판정) → 몽타주가 끝나면 종료
// 섹션이 여러 개면 콤보다. 콤보 구간(ComboOpen ~ ComboClose)에서 다시 발동하면 재발동(Retrigger)되어 다음 섹션이 나간다.
// 섹션이 하나뿐인 몽타주(AI 공격)는 한 번 치고 끝나는 단타가 된다.
UCLASS(Abstract)
class ELMCHRONICLES_API UGameplayAbility_Attack : public UElmGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Attack();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// Event.Attack.Hit 노티파이 시점에 호출된다. 자식 클래스가 "어떻게 맞출지"를 구현한다.
	virtual void OnAttackHit() {}

private:
	// 이번에 재생할 콤보 섹션 번호. InstancedPerActor라 활성화가 끝나도 값이 남는다.
	int32 mComboIndex = 0;

	// 콤보 입력 구간 안인지. true일 때 다시 공격하면 다음 타로 이어진다.
	bool mIsComboWindow = false;

private:
	UFUNCTION()
	void OnHitEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboOpen(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboClose(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();
};
