// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "../Player/MainMenuPlayerController.h"
#include "../Subsystem/SaveSubsystem.h"
#include "Engine/GameInstance.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// Pawn을 만들지 않는다. 엔진은 Pawn 만들기에 실패한 것으로 보고 컨트롤러를 Inactive 상태로 두는데, 메뉴(UI)에는 영향이 없다.
	// 화면은 맵의 MenuCamera가 맡는다. (ACameraActor::BeginPlay가 Player 0의 뷰 타깃으로 잡는다)
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();

	// 맵은 Soft 경로로만 들고 있다. (여는 것은 SaveSubsystem이 OpenLevel로 한다)
	mNewGameLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/ElmKingdom/KyleHouse.KyleHouse")));
	mNewGameStartTag = TEXT("Room");
}

bool AMainMenuGameMode::StartNewGame()
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();

	return SaveSubsystem && SaveSubsystem->StartNewGame(mNewGameLevel, mNewGameStartTag);
}
