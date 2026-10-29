// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffect_Recovery.h"
#include "../Attributes/BaseAttributeSet.h"
#include "../ElmGameplayTags.h"

UGameplayEffect_Recovery::UGameplayEffect_Recovery()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// HP
	FSetByCallerFloat HPCaller;
	HPCaller.DataTag = ElmTags::SetByCaller_Recovery_HP;

	FGameplayModifierInfo HPModifier;
	HPModifier.Attribute = UBaseAttributeSet::GetHPAttribute();
	HPModifier.ModifierOp = EGameplayModOp::Additive;
	HPModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(HPCaller);

	Modifiers.Add(HPModifier);

	// MP
	FSetByCallerFloat MPCaller;
	MPCaller.DataTag = ElmTags::SetByCaller_Recovery_MP;

	FGameplayModifierInfo MPModifier;
	MPModifier.Attribute = UBaseAttributeSet::GetMPAttribute();
	MPModifier.ModifierOp = EGameplayModOp::Additive;
	MPModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(MPCaller);

	Modifiers.Add(MPModifier);
}
