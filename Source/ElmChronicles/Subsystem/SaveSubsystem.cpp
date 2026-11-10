// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSubsystem.h"
#include "../Characters/PlayerCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
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

	// E를 누른 순간(문을 나선 순간)의 상태를 적는다. 어두워지는 동안 생긴 변화는 문을 나간 뒤라서 적지 않는다.
	WritePlayer(Player);

	// 어두워지는 동안 움직이거나 창을 열지 못하게 한다.
	// 캐릭터 입력(이동, 공격)과 컨트롤러 입력(E, 인벤토리)은 따로라서 둘 다 끈다.
	// 새 레벨에서는 둘 다 새로 만들어지므로 다시 켤 필요가 없다.
	if (APlayerController* PC = Player->GetController<APlayerController>())
	{
		Player->DisableInput(PC);
		PC->DisableInput(PC);
	}

	FadeOutAndOpen(MakeLevelURL(Level, StartTag));
}

bool USaveSubsystem::SaveToDisk(APlayerCharacter* Player)
{
	// 이동 중에는 메모에 "문을 나선 순간"의 상태가 적혀 있으므로 지금 상태로 덮어쓰지 않는다.
	// 죽은 상태도 저장하지 않는다. HP 0인 메모는 InitAttributes가 "죽어서 병원에 왔다"로 읽고 HP를 가득 채운다.
	if (mIsTraveling || !IsValid(Player) || Player->IsDead())
		return false;

	// 이어하기는 저장한 자리에 세운다. 점프 중에 저장하면 허공에서 시작해 떨어진다. (섬 사이였다면 끝없이 떨어진다)
	if (Player->GetCharacterMovement()->IsFalling())
		return false;

	// 포탈 이동과 같은 메모를 그대로 디스크에 쓴다.
	// 메모는 새 레벨이 초기화할 때만 읽으므로 지금 덮어써도 이번 레벨에는 영향이 없다.
	WritePlayer(Player);

	// 마지막 인자는 사용자 번호(콘솔에서 계정을 구분하는 용도). PC 싱글플레이는 0이다.
	return UGameplayStatics::SaveGameToSlot(mSaveGame, SaveSlotName, 0);
}

bool USaveSubsystem::StartNewGame(const TSoftObjectPtr<UWorld>& Level, FName StartTag)
{
	// 메인 화면 버튼을 빠르게 두 번 눌러도 한 번만 연다.
	if (mIsTraveling || Level.IsNull())
		return false;

	// 메모가 nullptr이면 새 레벨의 부품들이 각자 "새 게임"으로 초기화한다. (DT_Player 시작 레벨, 시작 아이템)
	// 디스크의 세이브 파일은 지우지 않는다. 게임 안에서 "저장하기"를 눌러야 덮어쓴다.
	mSaveGame = nullptr;

	FadeOutAndOpen(MakeLevelURL(Level, StartTag));
	return true;
}

bool USaveSubsystem::HasSaveOnDisk() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

bool USaveSubsystem::ContinueGame(const TSoftObjectPtr<UWorld>& FallbackLevel, FName FallbackStartTag)
{
	// 메인 화면 버튼을 빠르게 두 번 눌러도 한 번만 연다.
	if (mIsTraveling || FallbackLevel.IsNull())
		return false;

	// 파일이 없거나 깨졌으면 nullptr이다. (다른 클래스로 저장된 파일이면 Cast가 nullptr)
	UElmSaveGame* Loaded = Cast<UElmSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

	if (!Loaded)
		return false;

	// 읽은 세이브가 그대로 메모가 된다. 새 레벨의 부품들이 포탈로 온 것처럼 각자 초기화할 때 이 메모를 읽는다.
	// (능력치 = APlayerCharacter::InitAttributes, 가방 / 장비 = InitInventory, 퀘스트 = InitQuests)
	mSaveGame = Loaded;

	// 옛 세이브(버전 1)는 맵이 적혀 있지 않다. 맵 이름이 바뀌었으면 그 맵이 없다.
	// 없는 맵을 열면 검은 화면에서 멈추므로, 둘 다 새 게임 자리에서 메모의 능력치 / 가방으로 이어 간다.
	if (Loaded->LevelName.IsEmpty() || !FPackageName::DoesPackageExist(Loaded->LevelName))
	{
		UE_LOG(LogTestDebug, Warning, TEXT("ContinueGame : 저장한 맵을 쓸 수 없다(%s). 새 게임 자리에서 이어 간다."), *Loaded->LevelName);
		FadeOutAndOpen(MakeLevelURL(FallbackLevel, FallbackStartTag));
		return true;
	}

	// 저장한 맵은 태그 없이 연다. 도착한 맵의 AElmGameMode::RestartPlayer가 이 자리를 꺼내 PlayerStart 대신 쓴다.
	mContinueTransform = FTransform(Loaded->Rotation, Loaded->Location);
	FadeOutAndOpen(Loaded->LevelName);
	return true;
}

bool USaveSubsystem::ConsumeContinueTransform(FTransform& OutTransform)
{
	if (!mContinueTransform.IsSet())
		return false;

	OutTransform = mContinueTransform.GetValue();
	// 한 번만 쓴다. 남겨 두면 그 뒤의 포탈 이동도 태그를 무시하고 저장한 자리에 선다.
	mContinueTransform.Reset();
	return true;
}

void USaveSubsystem::WritePlayer(APlayerCharacter* Player)
{
	// 처음 적을 때만 만든다. UPROPERTY로 잡고 있어서 GameInstance가 살아 있는 동안 GC되지 않는다.
	if (!mSaveGame)
		mSaveGame = NewObject<UElmSaveGame>(this);

	// 지금 구조의 번호를 적는다. 헤더 기본값(1)은 "번호가 없는 옛 파일"을 읽을 때 쓰이므로 바꾸지 않는다.
	mSaveGame->SaveVersion = UElmSaveGame::CurrentVersion;

	Player->WriteToSave(*mSaveGame);
}

FString USaveSubsystem::MakeLevelURL(const TSoftObjectPtr<UWorld>& Level, FName StartTag)
{
	// URL "맵경로#태그"에서 # 뒤(Portal)를 엔진이 PlayerStart Tag로 쓴다.
	// 새 레벨의 GameMode가 FindPlayerStart에서 이 Tag를 가진 PlayerStart를 먼저 찾는다.
	FString URL = Level.GetLongPackageName();

	if (!StartTag.IsNone())
		URL += TEXT("#") + StartTag.ToString();

	return URL;
}

void USaveSubsystem::FadeOutAndOpen(const FString& URL)
{
	mIsTraveling = true;

	UE_LOG(LogTestDebug, Warning, TEXT("TravelToLevel : %s"), *URL);

	// 캐릭터가 없는 메인 화면에서도 부르므로 카메라와 타이머는 캐릭터 대신 GameInstance로 찾는다.
	// 투명(0) → 검정(1). 5번째 인자 true = 소리도 같이 줄인다. 6번째 인자 true = 다 어두워진 뒤에도 검정을 유지한다.
	// 유지하지 않으면 페이드가 끝나고 레벨이 열리기 전까지 한두 프레임 동안 화면이 번쩍 밝아질 수 있다.
	if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetGameInstance(), 0))
		Camera->StartCameraFade(0.f, 1.f, FadeTime, FLinearColor::Black, true, true);

	// 다 어두워지면 연다. URL은 Payload로 타이머에 실어 보낸다. (AssetSubsystem의 OnGameDataLoaded와 같은 방식)
	// GameInstance의 타이머는 월드 타이머와 같은 것이라(UWorld::GetTimerManager) 일시정지 중에는 돌지 않는다.
	FTimerHandle TimerHandle;
	GetGameInstance()->GetTimerManager().SetTimer(TimerHandle,
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
	if (!mIsTraveling || !IsValid(World) || World->GetGameInstance() != GetGameInstance())
		return;

	// 이어하기 자리는 이 맵의 첫 스폰(AElmGameMode::RestartPlayer)이 이미 꺼냈다.
	// GameMode가 AElmGameMode가 아닌 맵이라 꺼내지 않았어도, 다음 이동에 남지 않게 여기서 버린다.
	mContinueTransform.Reset();

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
