// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "../Save/ElmSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

class APlayerCharacter;

// 레벨을 옮겨도 사라지지 않는 플레이어 "메모"를 들고 있다.
// 레벨 이동(OpenLevel)은 PlayerState / 캐릭터를 모두 버리고 새로 만들지만, GameInstance와 그 서브시스템은 남는다.
// 이동 직전에 지금 상태를 메모에 적고, 새 레벨의 플레이어가 초기화할 때 메모를 읽는다.
// 나중에 디스크 저장도 이 메모(UElmSaveGame)를 SaveGameToSlot에 그대로 넘기면 된다.
// 화면 전환도 맡는다. 떠날 때 어둡게 하고(TravelToLevel), 도착하면 다시 밝힌다(OnPostLoadMap → FadeIn).
// 카메라는 레벨과 함께 사라지지만 서브시스템은 두 레벨 모두에 살아 있어서 출발과 도착을 이어 줄 수 있다.
UCLASS()
class ELMCHRONICLES_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	// nullptr이면 아직 한 번도 적지 않은 것(새 게임)이다.
	UPROPERTY() TObjectPtr<UElmSaveGame> mSaveGame;

	// 어두워지기 시작해서 새 레벨이 다시 밝아지기 시작할 때까지 true. 이 사이의 이동 요청은 무시한다.
	bool mIsTraveling = false;

	// 어두워지는 / 밝아지는 시간(초)
	static constexpr float FadeTime = 0.5f;
	// 도착한 뒤 밝히기 전에 검은 화면을 유지하는 시간(초). 이유는 OnPostLoadMap 주석.
	static constexpr float ArriveHoldTime = 0.5f;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 읽는 쪽은 nullptr이면 새 게임으로 시작한다.
	const UElmSaveGame* GetSaveGame() const { return mSaveGame; }

	// 지금 플레이어 상태를 메모에 적고, 화면을 어둡게 한 뒤 Level을 연다. StartTag는 도착 레벨 PlayerStart의 Player Start Tag.
	void TravelToLevel(APlayerCharacter* Player, const TSoftObjectPtr<UWorld>& Level, FName StartTag);

private:
	void WritePlayer(APlayerCharacter* Player);
	// 다 어두워지면 타이머가 부른다.
	void OpenLevelURL(FString URL);
	// 새 레벨이 다 열리면 엔진이 부른다. (FCoreUObjectDelegates::PostLoadMapWithWorld)
	void OnPostLoadMap(UWorld* World);
	// 도착하고 ArriveHoldTime이 지나면 타이머가 부른다.
	void FadeIn();
};
