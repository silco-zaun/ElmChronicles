// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIAttributeSet.h"
#include "MonsterAttributeSet.generated.h"

// 몬스터 전용 Attribute(처치 보상). FMonsterCharacterTableRow와 짝이다.
// AMonsterCharacter 생성자가 AAICharacter의 AttributeSet을 이 클래스로 바꿔 끼운다.
UCLASS()
class ELMCHRONICLES_API UMonsterAttributeSet : public UAIAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData	DropExp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData	DropGold;

public:
	ATTRIBUTE_FUNCTION(UMonsterAttributeSet, DropExp)
	ATTRIBUTE_FUNCTION(UMonsterAttributeSet, DropGold)
};
