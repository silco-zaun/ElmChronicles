// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffect_AbilityCost.h"
#include "../Attributes/BaseAttributeSet.h"
#include "../ElmGameplayTags.h"


UGameplayEffect_AbilityCost::UGameplayEffect_AbilityCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// AttributeSet이 가지고 있는 MP를 변경할 수 있게 한다.
	FGameplayModifierInfo MPModifierInfo;

	MPModifierInfo.Attribute = UBaseAttributeSet::GetMPAttribute();
	MPModifierInfo.ModifierOp = EGameplayModOp::Additive;

	// 감소시킬 마력은 고정값이 아니기 때문에 SetByCaller를 사용한다.
	FSetByCallerFloat MPCaller;

	MPCaller.DataTag = ElmTags::SetByCaller_Cost_MP;

	// SetByCaller를 ModifierInfo에 넣어준다.
	MPModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(MPCaller);

	Modifiers.Add(MPModifierInfo);

	// 체력
	FGameplayModifierInfo HPModifierInfo;

	HPModifierInfo.Attribute = UBaseAttributeSet::GetHPAttribute();
	HPModifierInfo.ModifierOp = EGameplayModOp::Additive;

	// 감소시킬 체력은 고정값이 아니기 때문에 SetByCaller를 사용한다.
	FSetByCallerFloat HPCaller;

	HPCaller.DataTag = ElmTags::SetByCaller_Cost_HP;

	// SetByCaller를 ModifierInfo에 넣어준다.
	HPModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(HPCaller);

	Modifiers.Add(HPModifierInfo);
}
