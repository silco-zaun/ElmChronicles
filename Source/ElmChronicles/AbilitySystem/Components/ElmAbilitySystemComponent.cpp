// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmAbilitySystemComponent.h"
#include "../Attributes/BaseAttributeSet.h"
#include "../../Data/Tables/LevelStatTableRow.h"

void UElmAbilitySystemComponent::InitLevelStats(int32 Level, const FLevelStatTableRow& Stat)
{
	// Max 값을 먼저 넣는다. (HP/MP는 Max를 넘지 않게 Clamp되므로, 순서가 바뀌면 0으로 잘린다)
	SetNumericAttributeBase(UBaseAttributeSet::GetLevelAttribute(), static_cast<float>(Level));
	SetNumericAttributeBase(UBaseAttributeSet::GetMaxHPAttribute(), Stat.MaxHP);
	SetNumericAttributeBase(UBaseAttributeSet::GetMaxMPAttribute(), Stat.MaxMP);
	SetNumericAttributeBase(UBaseAttributeSet::GetHPAttribute(), Stat.MaxHP);
	SetNumericAttributeBase(UBaseAttributeSet::GetMPAttribute(), Stat.MaxMP);
	SetNumericAttributeBase(UBaseAttributeSet::GetPhysicalAttackAttribute(), Stat.PhysicalAttack);
	SetNumericAttributeBase(UBaseAttributeSet::GetMagicalAttackAttribute(), Stat.MagicalAttack);
	SetNumericAttributeBase(UBaseAttributeSet::GetPhysicalDefenseAttribute(), Stat.PhysicalDefense);
	SetNumericAttributeBase(UBaseAttributeSet::GetMagicalDefenseAttribute(), Stat.MagicalDefense);
}

void UElmAbilitySystemComponent::InitCombatStats(float MoveSpeed, float AttackSpeed, float AttackRange, float AttackRadius)
{
	SetNumericAttributeBase(UBaseAttributeSet::GetMoveSpeedAttribute(), MoveSpeed);
	SetNumericAttributeBase(UBaseAttributeSet::GetAttackSpeedAttribute(), AttackSpeed);
	SetNumericAttributeBase(UBaseAttributeSet::GetAttackRangeAttribute(), AttackRange);
	SetNumericAttributeBase(UBaseAttributeSet::GetAttackRadiusAttribute(), AttackRadius);
}
