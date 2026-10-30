// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmGameModeBase.h"
#include "../AI/ElmAIController.h"
#include "../Characters/PlayerCharacter.h"
#include "../Player/ElmPlayerController.h"
#include "../Player/ElmPlayerState.h"
#include "../Subsystem/SaveSubsystem.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/GameInstance.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AElmGameModeBase::AElmGameModeBase()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AElmPlayerController::StaticClass();
	PlayerStateClass = AElmPlayerState::StaticClass();

	// 맵은 Soft 경로로만 들고 있다. (미리 불러 두지 않는다. 여는 것은 OpenLevel이 한다)
	mRespawnLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/ElmKingdom/Hospital.Hospital")));
	mRespawnStartTag = TEXT("Respawn");
	mRecallLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/ElmKingdom/KyleHouse.KyleHouse")));
	mRecallStartTag = TEXT("Room");
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

void AElmGameModeBase::PlayerKilled(APlayerCharacter* Player)
{
	// 쓰러지는 모습을 보여 준 뒤에 옮긴다.
	// 타이머가 불릴 때 캐릭터가 이미 사라졌을 수도 있어서 약한 참조(TWeakObjectPtr)로 실어 보낸다.
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle,
		FTimerDelegate::CreateUObject(this, &AElmGameModeBase::RespawnPlayer, TWeakObjectPtr<APlayerCharacter>(Player)),
		RespawnDelay, false);
}

void AElmGameModeBase::RespawnPlayer(TWeakObjectPtr<APlayerCharacter> Player)
{
	if (!Player.IsValid())
		return;

	// 화면 전환, 메모 적기, 레벨 열기는 포탈과 같은 길을 쓴다. 이미 포탈로 이동 중이면 TravelToLevel이 무시한다.
	// 메모에는 HP 0이 적히고, 병원에서 InitAttributes가 그것을 보고 HP / MP를 가득 채운 채로 둔다.
	if (USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>())
		SaveSubsystem->TravelToLevel(Player.Get(), mRespawnLevel, mRespawnStartTag);
}

bool AElmGameModeBase::IsInCombat(const APlayerCharacter* Player) const
{
	if (!IsValid(Player))
		return false;

	for (TActorIterator<AElmAIController> It(GetWorld()); It; ++It)
	{
		// 쓰러진 AI는 BT만 멈추고 Blackboard의 Target을 지우지 않는다. (AAICharacter::OnKilledBy)
		// 몬스터의 사망 애니메이션 동안, 필수 NPC가 쓰러져 있는 동안 Target이 남아 있으므로 건너뛴다.
		const APawn* AIPawn = It->GetPawn();
		if (!IsValid(AIPawn) || AElmAIController::IsDeadTarget(AIPawn))
			continue;

		if (It->GetTarget() == Player)
			return true;
	}

	return false;
}

void AElmGameModeBase::Recall(APlayerCharacter* Player)
{
	// 규칙을 지키는 곳은 GameMode다. 부르는 쪽(메뉴)이 확인을 빠뜨려도 막히도록 여기서 한 번 더 본다.
	if (!IsValid(Player) || Player->IsDead() || IsInCombat(Player))
		return;

	// 화면 전환, 메모 적기, 중복 이동 방지는 사망 후 병원 이동과 같은 길(TravelToLevel)을 쓴다.
	if (USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>())
		SaveSubsystem->TravelToLevel(Player, mRecallLevel, mRecallStartTag);
}
