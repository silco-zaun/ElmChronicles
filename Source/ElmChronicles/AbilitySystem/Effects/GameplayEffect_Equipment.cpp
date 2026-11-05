// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffect_Equipment.h"
#include "../Attributes/BaseAttributeSet.h"
#include "../ElmGameplayTags.h"

UGameplayEffect_Equipment::UGameplayEffect_Equipment()
{
	// Infinite : 지울 때까지 남는다. Modifier는 Base 값을 바꾸지 않고 그 위에 얹히므로,
	// 레벨업으로 Base 값이 바뀌어도 장비 수치는 그대로 더해진다.
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 네 Modifier가 모두 같은 모양(Additive + SetByCaller)이라 람다 하나로 만든다.
	auto AddSetByCallerModifier = [this](const FGameplayAttribute& Attribute, const FGameplayTag& DataTag)
		{
			FSetByCallerFloat Caller;
			Caller.DataTag = DataTag;

			FGameplayModifierInfo Modifier;
			Modifier.Attribute = Attribute;
			Modifier.ModifierOp = EGameplayModOp::Additive;
			Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Caller);

			Modifiers.Add(Modifier);
		};

	AddSetByCallerModifier(UBaseAttributeSet::GetPhysicalAttackAttribute(), ElmTags::SetByCaller_Equip_PhysicalAttack);
	AddSetByCallerModifier(UBaseAttributeSet::GetMagicalAttackAttribute(), ElmTags::SetByCaller_Equip_MagicalAttack);
	AddSetByCallerModifier(UBaseAttributeSet::GetPhysicalDefenseAttribute(), ElmTags::SetByCaller_Equip_PhysicalDefense);
	AddSetByCallerModifier(UBaseAttributeSet::GetMagicalDefenseAttribute(), ElmTags::SetByCaller_Equip_MagicalDefense);
}
