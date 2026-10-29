// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "GameFramework/GameModeBase.h"
#include "ElmGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class ELMCHRONICLES_API AElmGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AElmGameModeBase();

protected:
	// 포탈 태그 없이 들어왔을 때 설 PlayerStart를 고른다. 엔진 기본은 무작위라서 "태그 없는 PlayerStart"를 기본 자리로 정한다.
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
