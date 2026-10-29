// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAttributeSet.h"
#include "AIAttributeSet.generated.h"

// AI 캐릭터(몬스터, NPC) 공통 Attribute. FAICharacterTableRow와 짝이다.
// 몬스터 / NPC 전용 값은 자식인 UMonsterAttributeSet / UNPCAttributeSet에 둔다.
UCLASS()
class ELMCHRONICLES_API UAIAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AIAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData	DetectRange;

public:
	ATTRIBUTE_FUNCTION(UAIAttributeSet, DetectRange)
};
