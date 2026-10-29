// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAttributeSet.h"
#include "GameplayEffectExtension.h"

void UBaseAttributeSet::PreAttributeChange(
	const FGameplayAttribute& Attribute,	float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// HP나 MP는 MaxHP나 MaxMP를 초과할 수 없다.
	if (Attribute == GetHPAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHP());
	else if (Attribute == GetMPAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMP());
}

void UBaseAttributeSet::PreAttributeBaseChange(
	const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// Base 값도 Max를 넘지 않게 한다. 포션처럼 GE를 거치지 않는 변경도 모두 여기를 지난다.
	if (Attribute == GetHPAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHP());
	else if (Attribute == GetMPAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMP());
}

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// 임시 칸의 값을 꺼내고 바로 비운다.
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		const float OldHP = GetHP();

		// 이미 죽은 대상이면 무시한다. (같은 프레임에 여러 번 맞아도 사망 처리는 한 번)
		if (OldHP <= 0.f || Damage <= 0.f)
			return;

		const float NewHP = FMath::Clamp(OldHP - Damage, 0.f, GetMaxHP());
		SetHP(NewHP);

		// 엔진은 Context에 Instigator = OwnerActor, EffectCauser = AvatarActor를 넣는다.
		// 플레이어의 OwnerActor는 PlayerState이므로, 때린 "캐릭터"는 EffectCauser로 꺼낸다.
		AActor* Causer = Data.EffectSpec.GetEffectContext().GetEffectCauser();

		if (NewHP <= 0.f)
			mOnDeath.Broadcast(Causer);
		else
			mOnDamaged.Broadcast(Causer, Damage);
	}
}
