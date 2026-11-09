// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

// 메인 화면 맵(MainMenu) 전용 GameMode. 맵의 World Settings → GameMode Override로 지정한다.
// 메인 화면에는 조종할 캐릭터가 없다. 화면은 맵에 놓인 MenuCamera(Auto Activate for Player 0)가 보여 주고,
// 메뉴 위젯은 AMainMenuPlayerController가 띄운다.
// "어디서 시작하나"는 게임 규칙이라 여기서 정한다. (게임 맵의 AElmGameMode가 병원 / 귀환 자리를 정하는 것과 같다)
UCLASS()
class ELMCHRONICLES_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

protected:
	// 새 게임을 시작하는 맵(KyleHouse)과 PlayerStart 태그. 메뉴 "귀환"의 도착지와 같은 자리다.
	TSoftObjectPtr<UWorld> mNewGameLevel;
	FName mNewGameStartTag;

public:
	// 메인 화면 "새 게임". 이미 이동을 시작했으면 false를 돌려준다.
	bool StartNewGame();
};
