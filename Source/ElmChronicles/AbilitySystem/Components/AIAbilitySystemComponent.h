// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "ElmAbilitySystemComponent.h"
#include "AIAbilitySystemComponent.generated.h"

// AI 캐릭터(몬스터, NPC)의 ASC. PlayerState가 없으므로 AAICharacter가 Owner이자 Avatar다.
UCLASS()
class ELMCHRONICLES_API UAIAbilitySystemComponent : public UElmAbilitySystemComponent
{
	GENERATED_BODY()

};
