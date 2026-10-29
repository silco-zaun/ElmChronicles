// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSubsystem.h"
#include "../Characters/PlayerCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 레벨(맵)이 다 열릴 때마다 엔진이 부른다. 도착한 레벨의 화면을 밝히는 데 쓴다.
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USaveSubsystem::OnPostLoadMap);
}

void USaveSubsystem::Deinitialize()
{
	// 엔진 전역 알림이라 GameInstance가 사라질 때(PIE 종료 등) 직접 끊는다.
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

void USaveSubsystem::TravelToLevel(APlayerCharacter* Player, const TSoftObjectPtr<UWorld>& Level, FName StartTag)
{
	// 이미 이동 중이면 무시한다. 받아 주면 메모를 다시 적고 레벨을 두 번 연다.
	if (mIsTraveling || !IsValid(Player) || Level.IsNull())
		return;

	mIsTraveling = true;

	// E를 누른 순간(문을 나선 순간)의 상태를 적는다. 어두워지는 동안 생긴 변화는 문을 나간 뒤라서 적지 않는다.
	WritePlayer(Player);

	// URL "맵경로#태그"에서 # 뒤(Portal)를 엔진이 PlayerStart Tag로 쓴다.
	// 새 레벨의 GameMode가 FindPlayerStart에서 이 Tag를 가진 PlayerStart를 먼저 찾는다.
	FString URL = Level.GetLongPackageName();

	if (!StartTag.IsNone())
		URL += TEXT("#") + StartTag.ToString();

	UE_LOG(LogTestDebug, Warning, TEXT("TravelToLevel : %s"), *URL);

	// 어두워지는 동안 움직이거나 창을 열지 못하게 한다.
	// 캐릭터 입력(이동, 공격)과 컨트롤러 입력(E, 인벤토리)은 따로라서 둘 다 끈다.
	// 새 레벨에서는 둘 다 새로 만들어지므로 다시 켤 필요가 없다.
	if (APlayerController* PC = Player->GetController<APlayerController>())
	{
		Player->DisableInput(PC);
		PC->DisableInput(PC);
	}

	// 투명(0) → 검정(1). 5번째 인자 true = 소리도 같이 줄인다. 6번째 인자 true = 다 어두워진 뒤에도 검정을 유지한다.
	// 유지하지 않으면 페이드가 끝나고 레벨이 열리기 전까지 한두 프레임 동안 화면이 번쩍 밝아질 수 있다.
	if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(Player, 0))
		Camera->StartCameraFade(0.f, 1.f, FadeTime, FLinearColor::Black, true, true);

	// 다 어두워지면 연다. URL은 Payload로 타이머에 실어 보낸다. (AssetSubsystem의 OnGameDataLoaded와 같은 방식)
	FTimerHandle TimerHandle;
	Player->GetWorldTimerManager().SetTimer(TimerHandle,
		FTimerDelegate::CreateUObject(this, &USaveSubsystem::OpenLevelURL, URL), FadeTime, false);
}

void USaveSubsystem::OpenLevelURL(FString URL)
{
	// 0.5초 사이에 캐릭터가 사라졌을 수도 있으므로 World Context는 캐릭터 대신 GameInstance로 넘긴다.
	// OpenLevel은 다음 프레임에 지금 레벨을 통째로 버린다.
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(*URL));
}

void USaveSubsystem::OnPostLoadMap(UWorld* World)
{
	// 엔진 전역 알림이라 다른 GameInstance의 레벨(에디터 맵 열기 등)도 들어온다. 이동 중인 우리 레벨만 처리한다.
	if (!mIsTraveling || !IsValid(World) || World->GetGameInstance() != GetGameInstance())
		return;

	// LoadMap은 플레이어 스폰 → BeginPlay → 이 알림 순서라서 새 컨트롤러와 카메라가 이미 있다.
	// 새 카메라는 출발 레벨의 페이드를 모르므로 첫 화면이 그려지기 전에 검정으로 덮는다. (자동으로 변하지 않는 수동 페이드)
	if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(World, 0))
		Camera->SetManualCameraFade(1.f, FLinearColor::Black, true);

	// 곧바로 밝히지 않고 ArriveHoldTime 뒤에 밝힌다.
	// 로딩이 끝난 다음 프레임에는 로딩에 걸린 시간이 DeltaTime으로 한꺼번에 들어온다. (WorldSettings의 MaxUndilatedFrameTime, 최대 0.4초)
	// 그 프레임에 페이드 인이 돌고 있으면 0.5초짜리 페이드가 한 번에 80%까지 밝아져서 화면이 번쩍 켜진다.
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, this, &USaveSubsystem::FadeIn, ArriveHoldTime, false);
}

void USaveSubsystem::FadeIn()
{
	mIsTraveling = false;

	// 검정(1) → 투명(0). 마지막 인자 false라 다 밝아지면 페이드가 꺼진다.
	// 줄였던 소리 볼륨은 레벨을 옮겨도 그대로 남아 있다. 5번째 인자가 true여야 밝아지면서 소리도 원래 크기로 돌아온다.
	if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetGameInstance(), 0))
		Camera->StartCameraFade(1.f, 0.f, FadeTime, FLinearColor::Black, true, false);
}

void USaveSubsystem::WritePlayer(APlayerCharacter* Player)
{
	// 처음 적을 때만 만든다. UPROPERTY로 잡고 있어서 GameInstance가 살아 있는 동안 GC되지 않는다.
	if (!mSaveGame)
		mSaveGame = NewObject<UElmSaveGame>(this);

	Player->WriteToSave(*mSaveGame);
}
