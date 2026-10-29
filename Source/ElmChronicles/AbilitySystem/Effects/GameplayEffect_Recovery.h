// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "GameplayEffect.h"
#include "GameplayEffect_Recovery.generated.h"

// 포션 등 회복. HP/MP 회복량을 SetByCaller로 받아 더한다. (Max 초과는 AttributeSet이 막는다)
UCLASS()
class ELMCHRONICLES_API UGameplayEffect_Recovery : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Recovery();
};
