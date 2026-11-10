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
// 화면 전환도 맡는다. 떠날 때 어둡게 하고(FadeOutAndOpen), 도착하면 다시 밝힌다(OnPostLoadMap → FadeIn).
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
	// 이어하기로 연 맵에서 플레이어를 세울 자리. ContinueGame이 넣고, 도착한 맵의 첫 스폰(AElmGameMode::RestartPlayer)이 꺼내면서 지운다.
	// 비어 있으면 평소처럼 PlayerStart에 선다. (TOptional = "값이 있거나 없거나")
	TOptional<FTransform> mContinueTransform;
	// 어두워지는 / 밝아지는 시간(초)
	static constexpr float FadeTime = 0.5f;
	// 도착한 뒤 밝히기 전에 검은 화면을 유지하는 시간(초). 이유는 OnPostLoadMap 주석.
	static constexpr float ArriveHoldTime = 0.5f;
	// 디스크 저장 파일 이름. Saved/SaveGames/ElmSave.sav가 된다. 슬롯은 하나만 쓴다.
	static constexpr const TCHAR* SaveSlotName = TEXT("ElmSave");

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 읽는 쪽은 nullptr이면 새 게임으로 시작한다.
	const UElmSaveGame* GetSaveGame() const { return mSaveGame; }

	// 지금 플레이어 상태를 메모에 적고, 화면을 어둡게 한 뒤 Level을 연다. StartTag는 도착 레벨 PlayerStart의 Player Start Tag.
	void TravelToLevel(APlayerCharacter* Player, const TSoftObjectPtr<UWorld>& Level, FName StartTag);
	// 지금 플레이어 상태를 메모에 적고, 메모를 디스크(SaveSlotName)에 쓴다. 메뉴 "저장하기"가 부른다.
	// 이동 중이거나, 플레이어가 죽어 있거나 공중에 있으면 쓰지 않고 false를 돌려준다.
	bool SaveToDisk(APlayerCharacter* Player);
	// 메인 화면 "새 게임".
	// 메모를 비우고(nullptr = 새 게임) 화면을 어둡게 한 뒤 Level을 연다.
	// 이미 이동 중이면 무시하고 false를 돌려준다.
	bool StartNewGame(const TSoftObjectPtr<UWorld>& Level, FName StartTag);
	// 디스크에 세이브 파일이 있는지. 메인 화면 "이어하기" 버튼을 켤지 정한다.
	bool HasSaveOnDisk() const;
	// 메인 화면 "이어하기". 디스크의 세이브를 메모로 읽고, 저장한 맵의 저장한 자리로 간다.
	// 맵이 적혀 있지 않거나(옛 세이브) 그 맵이 없어졌으면 FallbackLevel의 FallbackStartTag 자리에서 메모의 능력치 / 가방으로 이어 간다.
	// 이미 이동 중이거나 세이브를 읽지 못하면 false를 돌려준다.
	bool ContinueGame(const TSoftObjectPtr<UWorld>& FallbackLevel, FName FallbackStartTag);
	// 이어하기로 도착한 첫 스폰 자리를 꺼낸다. 한 번 꺼내면 지운다. 없으면 false. (AElmGameMode::RestartPlayer가 부른다)
	bool ConsumeContinueTransform(FTransform& OutTransform);

private:
	void WritePlayer(APlayerCharacter* Player);
	// "맵경로#태그"를 만든다. # 뒤는 도착 레벨에서 PlayerStart Tag로 쓰인다.
	static FString MakeLevelURL(const TSoftObjectPtr<UWorld>& Level, FName StartTag);
	// 화면을 어둡게 하고, 다 어두워지면 URL을 연다.
	// 포탈 / 귀환 / 사망 / 새 게임 / 이어하기가 같이 쓴다.
	void FadeOutAndOpen(const FString& URL);
	// 다 어두워지면 타이머가 부른다.
	void OpenLevelURL(FString URL);
	// 새 레벨이 다 열리면 엔진이 부른다. (FCoreUObjectDelegates::PostLoadMapWithWorld)
	void OnPostLoadMap(UWorld* World);
	// 도착하고 ArriveHoldTime이 지나면 타이머가 부른다.
	void FadeIn();
};
