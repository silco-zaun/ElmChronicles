// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "../../GameInfo.h"
#include "AbilitySystemComponent.h"
#include "ElmAbilitySystemComponent.generated.h"

struct FLevelStatTableRow;

// 플레이어와 AI(몬스터, NPC) ASC의 공통 부모.
UCLASS()
class ELMCHRONICLES_API UElmAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// 레벨 스탯 테이블 값으로 기본 능력치를 채운다.
	void InitLevelStats(int32 Level, const FLevelStatTableRow& Stat);

	// 이동/공격 관련 공통 능력치를 채운다.
	void InitCombatStats(float MoveSpeed, float AttackSpeed, float AttackRange, float AttackRadius);
};
