// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayAbility_Attack.h"
#include "GameplayAbility_Melee.generated.h"

// 앞쪽으로 구를 쓸어서(Sweep) 맞은 대상에게 데미지를 주는 근접 공격. 플레이어 / 몬스터 / NPC가 모두 쓴다.
UCLASS()
class ELMCHRONICLES_API UGameplayAbility_Melee : public UGameplayAbility_Attack
{
	GENERATED_BODY()

protected:
	virtual void OnAttackHit() override;
};
