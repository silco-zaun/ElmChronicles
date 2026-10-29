// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffect_Reward.h"
#include "../Attributes/PlayerAttributeSet.h"
#include "../ElmGameplayTags.h"

UGameplayEffect_Reward::UGameplayEffect_Reward()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// 경험치
	FSetByCallerFloat ExpCaller;
	ExpCaller.DataTag = ElmTags::SetByCaller_Reward_Exp;

	FGameplayModifierInfo ExpModifier;
	ExpModifier.Attribute = UPlayerAttributeSet::GetExpAttribute();
	ExpModifier.ModifierOp = EGameplayModOp::Additive;
	ExpModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(ExpCaller);

	Modifiers.Add(ExpModifier);

	// 골드
	FSetByCallerFloat GoldCaller;
	GoldCaller.DataTag = ElmTags::SetByCaller_Reward_Gold;

	FGameplayModifierInfo GoldModifier;
	GoldModifier.Attribute = UPlayerAttributeSet::GetGoldAttribute();
	GoldModifier.ModifierOp = EGameplayModOp::Additive;
	GoldModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(GoldCaller);

	Modifiers.Add(GoldModifier);
}
