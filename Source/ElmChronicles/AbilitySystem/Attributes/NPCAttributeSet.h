// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIAttributeSet.h"
#include "NPCAttributeSet.generated.h"

// NPC 전용 Attribute. FNPCCharacterTableRow와 짝이다.
// 지금은 비어 있다. NPC만 갖고 GE로 바뀌는 숫자(예: 호감도)가 생기면 여기에 둔다.
// IsEssential / ReviveTime / DialogueRow처럼 GE로 바꾸지 않는 값은 ANPCCharacter의 멤버로 둔다.
// ANPCCharacter 생성자가 AAICharacter의 AttributeSet을 이 클래스로 바꿔 끼운다.
UCLASS()
class ELMCHRONICLES_API UNPCAttributeSet : public UAIAttributeSet
{
	GENERATED_BODY()

};
