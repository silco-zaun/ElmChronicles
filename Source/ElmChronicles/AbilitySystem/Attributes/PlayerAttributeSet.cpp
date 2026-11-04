// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// 데미지(IncomingDamage)는 부모가 처리한다.
	Super::PostGameplayEffectExecute(Data);

	// GE를 거친 변경(보상)에만 반응한다. 레벨 이동 때 메모 값을 SetNumericAttributeBase로 넣는 것은
	// 여기를 지나지 않으므로, 맵을 옮길 때마다 레벨업 판정이 다시 돌지 않는다.
	if (Data.EvaluatedData.Attribute == GetExpAttribute())
		mOnExpGained.Broadcast();
}
