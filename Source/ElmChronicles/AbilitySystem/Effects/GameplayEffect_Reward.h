// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "GameplayEffect.h"
#include "GameplayEffect_Reward.generated.h"

// 몬스터 처치 보상. 경험치와 골드를 SetByCaller로 받아 더한다.
UCLASS()
class ELMCHRONICLES_API UGameplayEffect_Reward : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Reward();
};
