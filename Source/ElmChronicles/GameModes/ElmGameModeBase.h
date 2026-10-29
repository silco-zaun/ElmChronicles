// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "GameFramework/GameModeBase.h"
#include "ElmGameModeBase.generated.h"

class APlayerCharacter;

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
	// 죽으면 다시 시작하는 맵(병원)과 그 맵에서 설 PlayerStart의 Player Start Tag.
	// 어느 맵에서 죽어도 같은 자리로 가므로 포탈의 "출발 맵 이름" 규칙 대신 고정 태그를 쓴다.
	TSoftObjectPtr<UWorld> mRespawnLevel;
	FName mRespawnStartTag;

	// 쓰러진 뒤 화면이 어두워지기 시작할 때까지의 시간(초). 그동안 쓰러지는 애니메이션을 보여 준다.
	static constexpr float RespawnDelay = 2.f;

protected:
	// 포탈 태그 없이 들어왔을 때 설 PlayerStart를 고른다. 엔진 기본은 무작위라서 "태그 없는 PlayerStart"를 기본 자리로 정한다.
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

public:
	// 플레이어 캐릭터가 죽으면 캐릭터가 부른다. RespawnDelay 뒤에 병원으로 보낸다.
	void PlayerKilled(APlayerCharacter* Player);

private:
	// 타이머가 부른다. 그 사이에 캐릭터가 사라졌을 수 있어서 약한 참조로 받는다. (타이머 Payload라 값으로 받는다)
	void RespawnPlayer(TWeakObjectPtr<APlayerCharacter> Player);
};
