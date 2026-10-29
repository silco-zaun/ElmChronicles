// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmGameModeBase.h"
#include "../Characters/PlayerCharacter.h"
#include "../Player/ElmPlayerController.h"
#include "../Player/ElmPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"

AElmGameModeBase::AElmGameModeBase()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AElmPlayerController::StaticClass();
	PlayerStateClass = AElmPlayerState::StaticClass();
}

AActor* AElmGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// 포탈 태그(URL의 #태그)로 찾는 일은 이 함수보다 먼저 FindPlayerStart가 끝냈다.
	// 여기까지 왔다는 것은 태그 없이 들어왔거나, 그 태그를 가진 PlayerStart가 없다는 뜻이다.
	APlayerStart* DefaultStart = nullptr;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		// 뷰포트 우클릭 "Play From Here"는 엔진이 그 자리에 APlayerStartPIE를 임시로 만든다. 테스트할 때는 이것이 가장 먼저다.
		if (It->IsA<APlayerStartPIE>())
			return *It;

		// 태그가 있는 PlayerStart는 포탈 도착 지점이라 이름으로 불렸을 때만 쓴다. 태그 없는 첫 PlayerStart가 기본 자리다.
		if (!DefaultStart && It->PlayerStartTag.IsNone())
			DefaultStart = *It;
	}

	if (DefaultStart)
		return DefaultStart;

	// 태그 없는 PlayerStart가 없는 맵(포탈로만 들어오는 방 등)은 엔진 방식(빈 자리 중 무작위)을 쓴다.
	return Super::ChoosePlayerStart_Implementation(Player);
}
