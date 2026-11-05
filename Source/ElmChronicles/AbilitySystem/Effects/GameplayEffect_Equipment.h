// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "GameplayEffect.h"
#include "GameplayEffect_Equipment.generated.h"

// 장비 능력치. 공격력 / 마법 공격력 / 방어력 / 마법 방어력 증가량을 SetByCaller로 받아 더한다.
// 벗을 때까지 남는 Infinite GE라서, 장비칸마다 하나씩 걸고 벗거나 바꿀 때 그 Handle로 지운다. (InventoryComponent)
UCLASS()
class ELMCHRONICLES_API UGameplayEffect_Equipment : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Equipment();
};
