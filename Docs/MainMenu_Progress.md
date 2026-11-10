# 메인 화면 (새 게임 / 이어하기 / 옵션 / 게임 종료)

> **다음 세션 시작 방법**: 이 문서를 읽고 → `git status` / `git diff`로 어디까지 했는지 확인하고 → [진행 상황](#진행-상황) 표에서 첫 번째 ☐ / ◐ 단계부터 이어서 한다.

## 목표
게임을 켜면 `/Game/Maps/MainMenu` 맵이 배경으로 보이고, 그 위에 화면 전체를 덮는 메뉴(새 게임 / 이어하기 / 옵션 / 게임 종료)가 뜬다. 새 게임은 카일의 집에서 시작하고, 이어하기는 디스크에 저장한 맵과 자리에서 이어진다. 옵션 창에서는 화면 모드 / 해상도, 그래픽 품질, 마우스 감도를 바꾼다.

## 배경
- 2026-11-08 사용자 요청: "Stack O Bot과 같은 방식"으로 만든다. Stack O Bot(`c:\Tadi\Work\StackOBot`)의 구성은 이렇다(에셋 문자열로 확인).
  - `LVL_MainMenu`의 World Settings가 GameMode를 `GM_MainMenu`(GameModeBase)로 바꾼다.
  - `GM_MainMenu`의 HUD Class `HUD_MainMenu`가 BeginPlay에서 `UI_MainMenu`를 `AddToViewport`하고, 입력 모드(GameAndUI)와 커서를 맞춘다.
  - `UI_MainMenu`: Overlay 루트 + 배경 + Vertical Box 안의 버튼(`UI_Button` 재사용 위젯). Start는 `LVL_StackOBot`을 열고, Quit은 `QuitGame`을 부른다.
- 이 프로젝트에서 이미 있는 것:
  - `MainMenu` 맵(2026-11-06): Field 성 안쪽 복사본, 표시용 `Kyle`(SkeletalMeshActor), `MenuCamera`(Auto Activate for Player 0). GameMode Override가 없어서 지금은 게임용 `AElmGameMode`가 돌고, HUD와 게임 캐릭터가 생긴다.
  - 레벨 이동 / 메모 / 디스크 저장: `USaveSubsystem`(`TravelToLevel`, `SaveToDisk`). 디스크에 쓰기만 하고 불러오기는 없다. `UElmSaveGame`에 맵과 위치가 없다.
  - 게임 안 메뉴(B): `UMenuWidget`. 버튼은 일을 맡은 곳(GameMode, SaveSubsystem)에 부탁만 한다. 메인 화면도 이 규칙을 따른다.

## 결정 사항

| 항목 | 결정 | 이유 / 버린 안 |
|---|---|---|
| 구성 | MainMenu 맵의 World Settings GameMode Override = `AMainMenuGameMode`(Pawn 없음). `AMainMenuPlayerController`가 `WBP_MainMenu`를 띄우고 UI 전용 입력 + 커서로 맞춘다. 배경은 맵의 `MenuCamera` | Stack O Bot은 HUD 클래스가 위젯을 띄우지만, 이 프로젝트는 게임 HUD도 컨트롤러가 만들므로(`AElmPlayerController`) 맞췄다. |
| 메뉴 버튼이 하는 일 | 위젯 → `AMainMenuGameMode`(어디서 시작하나 = 규칙) → `USaveSubsystem`(메모, 페이드, 레벨 열기) | 게임 안 메뉴의 귀환(위젯 → `AElmGameMode::Recall` → `TravelToLevel`)과 같은 길. |
| 새 게임 시작 자리 | KyleHouse의 `Room` PlayerStart(메뉴 "귀환"의 도착지와 같다) | 2026-11-08 사용자 선택. 버린 안: Field 기본 자리. |
| 새 게임과 세이브 파일 | 메모만 비운다(`mSaveGame = nullptr`). 디스크의 `ElmSave.sav`는 지우지 않고, 게임 안에서 "저장하기"를 눌러야 덮어쓴다. 확인 상자도 없다 | 파일을 건드리지 않으므로 물을 이유가 없다. |
| 이어하기 자리 | **저장한 맵 + 그 자리.** `UElmSaveGame`에 `LevelName` / `Location` / `Rotation`을 더하고, 도착한 맵의 `AElmGameMode::RestartPlayer`가 PlayerStart 대신 그 자리에 세운다 | 2026-11-08 사용자 선택. 버린 안: 저장한 맵의 기본 자리(실내 맵에는 태그 없는 PlayerStart가 없어 아무 데나 선다), 항상 카일의 집. |
| 옛 세이브(맵 없음) | `LevelName`이 비어 있으면 새 게임 자리(KyleHouse `Room`)에서 메모의 능력치 / 가방으로 이어 간다 | 2단계 전에 만든 세이브(버전 1)도 버리지 않는다. |
| 이어하기 버튼 | 세이브 파일이 없으면 끈다(`DoesSaveGameExist`). 읽기에 실패해도 끈다 | |
| 저장한 맵이 없을 때 | `FPackageName::DoesPackageExist`로 확인하고, 없으면 옛 세이브처럼 새 게임 자리에서 메모의 능력치 / 가방으로 이어 간다 | 맵 이름은 바뀔 수 있다(2026-11-09 필드 맵이 `ElmKingdom`이 됐다). 없는 맵을 열면 검은 화면에서 멈춘다. `DoesPackageExist`는 패키지 게임(IoStore)에서도 확인한다(엔진 `PackageName.cpp` `InternalDoesPackageExistEx`). |
| 공중에서 저장 | `SaveToDisk`가 떨어지는 중(`IsFalling`)이면 거절한다(메뉴에는 기존 문구 "저장하지 못했습니다.") | 이어하기는 저장한 자리에 세운다. 섬 사이 점프 중에 저장하면 허공에서 시작해 끝없이 떨어진다(KillZ 처리 미구현). 2026-11-09 Claude 제안. |
| 세이브 버전 번호 | 헤더 기본값은 1(가장 옛 버전) 그대로 두고, `USaveSubsystem::WritePlayer`가 적을 때 `UElmSaveGame::CurrentVersion`(2)을 넣는다. 옛 세이브인지는 버전이 아니라 `LevelName`이 비었는지로 판단한다 | 주의 14 |
| 이어하기 자리 지우기 | `ConsumeContinueTransform`이 꺼내면서 지우고, 남은 것은 `OnPostLoadMap`이 버린다(도착한 맵의 GameMode가 `AElmGameMode`가 아니라서 꺼내지 않은 경우) | 2026-11-10 변경. 처음 안은 `StartNewGame`에서 지우기였는데, 그러면 새 게임 길 하나만 막는다. LoadMap은 플레이어 스폰(`SpawnPlayActor` → `RestartPlayer`) → `BeginPlay` → `PostLoadMapWithWorld` 순서라서(엔진 `UnrealEngine.cpp` `UEngine::LoadMap`) 도착 알림 때 지우면 어느 맵이든 다음 이동에 남지 않는다. |
| 게임 종료 | 메인 화면에서는 묻지 않고 바로 끈다 | 잃을 진행이 없다. 게임 안 메뉴의 확인 상자(`UConfirmWidget`)는 `AElmPlayerController`에 묶여 있어서 그대로 쓸 수 없다. |
| 옵션 항목 | 화면 모드 + 해상도, 그래픽 품질(Scalability 전체 4단계), 마우스 감도 | 2026-11-08 사용자 선택. 소리 볼륨은 프로젝트에 소리 에셋이 하나도 없어서 뺐다. VSync는 고르지 않았다. |
| 옵션 저장 | 엔진 `UGameUserSettings`(GameUserSettings.ini). 마우스 감도는 엔진에 없는 값이라 자식 클래스 `UElmGameUserSettings`에 `Config` 프로퍼티로 둔다 | 세이브(`ElmSave.sav`)는 게임 진행용이라 섞지 않는다. 새 게임을 해도 옵션은 남아야 한다. |
| 마우스 감도 값 | 배율(기본 1.0). 최종 감도 = `APlayerCharacter::mMouseSensitivity`(지금 0.5, 개발자가 맞춘 기본 감도) × 옵션 배율 | 기본 감도를 코드 한 곳에 남겨 둔다. |
| 옵션 창 위치 | `WBP_Options`를 `WBP_MainMenu` 안에 두고(화면 전체, 반투명 배경) 열고 닫는다. 메인 화면에 의존하지 않게 만든다 | 나중에 B 메뉴에서도 쓸 수 있다. 이번 범위에서는 연결하지 않는다(2026-11-08 사용자 선택). |
| 메뉴 위치 | 화면 오른쪽 가운데 | MainMenu 맵 구도(카메라 (-10000, -175, 210), `Kyle` (-9100, -300))에서 Kyle이 화면 가로 약 38%, 가운데보다 왼쪽에 선다. PIE에서 겹치면 `MenuBox` 앵커만 바꾼다. |
| 떠날 때 메뉴 모습 | 버튼을 누르면 클릭을 막고(HitTestInvisible) `WBP_MainMenu`의 `FadeOut` 애니메이션(0.5초)으로 메뉴를 지운다 | 아래 주의 1. |

## 주의

1. **카메라 페이드는 UMG를 덮지 않는다.** `StartCameraFade`는 3D 화면만 어둡게 한다(CLAUDE.md "카메라 페이드라 HUD(UMG)는 가려지지 않는다"). 메뉴를 그대로 두면 검은 화면에 버튼만 떠 있다. 그래서 `UMainMenuWidget::HideForTravel`이 위젯을 함께 지운다. `FadeOut` 애니메이션이 없으면 `PlayWidgetAnimation`이 아무것도 하지 않아서 동작은 같고 모습만 어색하다.
   - **WBP 루트의 기본 Render Opacity가 0으로 저장되면 메뉴가 처음부터 보이지 않는다**(2026-11-09). 버튼은 투명한 채로 눌리고, 새 게임을 누르면 `FadeOut`(0초 = 1)이 재생되면서 그제야 메뉴가 잠깐 보였다가 사라진다. `FadeOut` 키를 만들다가 트랙 키 대신 위젯 기본값을 0으로 바꾼 것이다. 고칠 때는 애니메이션 선택을 풀고 루트 Canvas Panel의 Render Opacity를 1로 되돌린다. 애니메이션이 선택된 동안에는 Details에서 값을 바꾸면 키가 찍힌다. 확인은 unreal-mcp `get_properties`(`/Game/UI/MainMenu/WBP_MainMenu.WBP_MainMenu_C:WidgetTree.<루트 이름>`의 `RenderOpacity`)로 한다.
   - 애니메이션은 이름으로 찾는다. `UElmBaseWidget::NativeOnInitialized`가 WBP 애니메이션 객체 이름에서 `_INST`를 뗀 문자열을 키로 `mAnimMap`에 넣으므로, WBP의 애니메이션 이름이 정확히 `FadeOut`이어야 한다. `UMainMenuWidget::NativeOnInitialized`는 `Super`를 먼저 불러야 맵이 채워진다.
2. **`DefaultPawnClass = nullptr`이면 엔진은 Pawn 만들기에 실패한 것으로 본다.** `RestartPlayerAtPlayerStart` → `FailedToRestartPlayer` → `APlayerController::FailedToSpawnPawn`이 컨트롤러를 `NAME_Inactive` 상태로 둔다(엔진 `GameModeBase.cpp`, `PlayerController.cpp`). 경고 로그는 없고 UI 입력에는 영향이 없다. 화면은 `ACameraActor::BeginPlay`가 Player 0의 뷰 타깃으로 잡는 `MenuCamera`가 보여 준다.
3. **`WBP_MainMenu`는 C++ 빌드 뒤에만 만들 수 있다.** 부모가 `UMainMenuWidget`이기 때문이다. 그런데 `AMainMenuPlayerController` 생성자가 `ConstructorHelpers`로 이 WBP를 찾으므로, 빌드 직후 에디터를 처음 열 때는 `CDO Constructor (MainMenuPlayerController): Failed to find ... WBP_MainMenu` 에러가 한 번 난다(정상). WBP를 만들고 저장한 뒤 **에디터를 다시 열어야** CDO가 WBP를 찾는다.
4. **화면에 붙인 위젯은 레벨이 바뀔 때 엔진이 뗀다.** `UGameViewportSubsystem::HandleWorldCleanup`(`bAutoRemoveOnWorldRemoved`). 그래서 `AMainMenuPlayerController`에는 `EndPlay`가 없다.
5. **PIE에서는 월드 패키지 이름에 `UEDPIE_0_`이 붙는다**(`/Game/Maps/ElmKingdom/UEDPIE_0_ElmKingdom`). 이어하기용 맵 이름은 `UWorld::RemovePIEPrefix`로 떼고 적는다(2단계). 패키지 게임에서는 붙지 않는다.
6. **이어하기 자리는 한 번만 쓴다.** `USaveSubsystem`이 "이어하기로 도착한 첫 스폰"용 Transform을 들고 있다가 `AElmGameMode::RestartPlayer`가 꺼내면서 지운다. 지우지 않으면 그 뒤의 포탈 이동도 태그를 무시하고 저장한 자리에 선다. `RestartPlayerAtTransform`은 `FindPlayerStart`를 거치지 않는다.
7. **창 모드 / 해상도는 PIE에서 바뀌지 않는다.** 해상도 요청(`FSystemResolution`)을 처리하는 곳이 `UGameEngine`이고(`GameEngine.cpp`), 에디터 PIE는 `UEditorEngine`이다. Standalone Game이나 패키지에서 확인한다.
8. **PIE에서 그래픽 품질을 바꾸면 에디터 뷰포트 품질도 바뀐다.** Scalability CVar를 에디터와 함께 쓰고, 에디터에서는 `GEditorSettingsIni`에 저장된다(`UGameUserSettings::SaveSettings`). 테스트 뒤 에디터 뷰포트의 Scalability를 원래대로 돌려 둔다.
9. **"테두리 없는 창"에서는 해상도 값이 무시된다.** 엔진이 바탕화면 해상도를 쓴다(`GameUserSettings.cpp`의 `WindowedFullscreen ? GetDesktopResolution()`). 그래서 이 모드에서는 해상도 콤보박스를 끈다.
10. **`GetOverallScalabilityLevel()`은 항목별 품질이 섞여 있으면 -1을 돌려준다.** 그때는 품질 콤보박스를 비워 두고, 적용할 때도 고르지 않았으면 바꾸지 않는다.
11. **Game User Settings Class는 에디터를 다시 열어야 바뀐다**(`UEngine::GameUserSettingsClassName`이 `ConfigRestartRequired`). 설정 전에는 `UGameUserSettings::GetGameUserSettings()`가 엔진 기본 클래스라 `Cast<UElmGameUserSettings>`가 nullptr이다. 읽는 쪽(`LookKey`)은 nullptr이면 배율 1로 쓴다.
12. **2026-11-09 기준 디스크 세이브가 없다**(`Saved/SaveGames` 폴더 없음. 2026-11-10 `C:\ElmChronicles`에도 없다). 2단계를 빌드하기 전에 지금 빌드로 B → "저장하기"를 하면 버전 1(맵 없음) 세이브가 생긴다. 2단계의 "옛 세이브" 확인에 쓰도록 `ElmSave.sav`를 다른 이름으로 복사해 둔다(2단계 빌드로 저장하면 덮어쓴다).
13. **2026-11-09에 사용자가 이름을 바꿨다**(이 계획 밖에서 같이 한 일).
    - 게임 GameMode: `AElmGameModeBase` → `AElmGameMode`(`GameModes/ElmGameMode.h`, `GlobalDefaultGameMode`도 바꿈). 내용은 이름만 다르다. 백업 / 테스트 맵 3개(`ElmKingdom/Field_Backup`, `ElmKingdom/Field_Old`, `Test`)에는 옛 클래스 이름이 남아 있다. ini에 `CoreRedirects`가 없어서, 이 맵들을 열거나 쿠킹하면 옛 클래스를 찾지 못한다(GameMode Override가 비면 전역 기본값 `AElmGameMode`로 돈다).
    - 필드 맵: 이제 `/Game/Maps/ElmKingdom/ElmKingdom`(`EditorStartupMap`, KyleHouse 포탈 도착 `ElmKingdom#HeroHouse`). `Field.umap`은 없다.
    - 메인 화면 맵: `MainMenu_Change`를 `MainMenu`로 이름 바꾸고(옛 맵은 `MainMenu_Backup`), 1.3 KB짜리 리다이렉터 `MainMenu_Change.umap`이 남았다. `GameDefaultMap`이 이 리다이렉터를 가리킨다. 지금은 엔진이 리다이렉터를 따라가서 열리지만, Fix Up Redirectors로 리다이렉터가 지워지면 시작 맵을 잃는다.
14. **SaveGame 파일에는 기본값과 같은 값이 쓰이지 않는다.** `UGameplayStatics::SaveGameToMemory`가 `ArNoDelta` 없이 `Serialize`하므로, CDO(헤더 기본값)와 같은 프로퍼티는 파일에서 빠지고 읽을 때 그때의 기본값이 들어간다(엔진 `GameplayStatics.cpp`). 그래서 헤더의 `SaveVersion` 기본값을 2로 바꾸면 버전 1 파일도 2로 읽힌다(1단계까지 만든 파일에는 `SaveVersion`이 아예 없다). 기본값은 1로 두고, 쓰는 쪽(`WritePlayer`)이 현재 번호를 넣는다. 다른 필드도 헤더 기본값을 바꾸면 옛 세이브에서 읽히는 값이 바뀐다.
15. **저장한 자리가 맞는지는 확인하지 않는다.** 저장한 뒤 맵을 고쳐서 그 자리가 허공이 되면 그대로 떨어진다. 벽 속이 되면 Pawn 기본값 `AdjustIfPossibleButDontSpawnIfColliding`(엔진 `Pawn.cpp`)으로 엔진이 조금 밀어 보고, 밀 자리가 없으면 캐릭터를 만들지 못한다(`SpawnDefaultPawnAtTransform` 경고 → `FailedToRestartPlayer`, 조종할 캐릭터 없음). 맵을 크게 고쳤으면 테스트용 세이브를 다시 만든다.

## 목표 구조

```cpp
// GameModes/MainMenuGameMode.h  (MainMenu 맵 World Settings → GameMode Override)
TSoftObjectPtr<UWorld> mNewGameLevel;   // KyleHouse
FName mNewGameStartTag;                 // "Room"
bool StartNewGame();                    // → USaveSubsystem::StartNewGame
bool ContinueGame();                    // (2단계) → USaveSubsystem::ContinueGame(mNewGameLevel, mNewGameStartTag)

// Player/MainMenuPlayerController.h
// BeginPlay: WBP_MainMenu AddToViewport → FInputModeUIOnly(DoNotLock) → 커서 표시

// UI/MainMenu/MainMenuWidget.h  (WBP_MainMenu의 부모)
// BindWidget: mNewGameButton, mContinueButton, mOptionsButton, mQuitButton, mOptionsWidget(3단계)
void HideForTravel();                   // HitTestInvisible + "FadeOut" 애니메이션

// Subsystem/SaveSubsystem.h
bool StartNewGame(const TSoftObjectPtr<UWorld>& Level, FName StartTag);  // 메모 비우기 + FadeOutAndOpen
bool HasSaveOnDisk() const;                                                // (2단계)
bool ContinueGame(const TSoftObjectPtr<UWorld>& FallbackLevel, FName FallbackStartTag); // (2단계) LoadGameFromSlot → 메모
bool ConsumeContinueTransform(FTransform& OutTransform);                   // (2단계) 한 번 꺼내면 지운다
TOptional<FTransform> mContinueTransform;                                  // (2단계) ContinueGame이 넣고, RestartPlayer가 꺼내고, OnPostLoadMap이 남은 것을 버린다
static FString MakeLevelURL(const TSoftObjectPtr<UWorld>& Level, FName StartTag); // "맵경로#태그"
void FadeOutAndOpen(const FString& URL);  // 포탈 / 귀환 / 사망 / 새 게임 / 이어하기가 같이 쓴다
// 1단계에서 나누는 방법:
//   TravelToLevel  = 가드(mIsTraveling / Player / Level) → WritePlayer → 캐릭터·컨트롤러 DisableInput → FadeOutAndOpen(MakeLevelURL(...))
//   StartNewGame   = 가드(mIsTraveling / Level) → mSaveGame = nullptr → FadeOutAndOpen(...) → true
//   FadeOutAndOpen = mIsTraveling = true → 로그 "TravelToLevel : <URL>"(로그 문구는 그대로 둔다)
//                    → GetPlayerCameraManager(GetGameInstance(), 0)->StartCameraFade(0, 1, FadeTime, Black, true, true)
//                    → GetGameInstance()->GetTimerManager()로 FadeTime 뒤 OpenLevelURL(URL)
//   (카메라 / 타이머를 캐릭터가 아니라 GameInstance로 찾는 것이 핵심이다. 도착 쪽 OnPostLoadMap / FadeIn은 바꾸지 않는다)

// Save/ElmSaveGame.h  (2단계. SaveVersion 기본값은 1 그대로, static constexpr int32 CurrentVersion = 2를 WritePlayer가 넣는다. 주의 14)
FString LevelName;   // "/Game/Maps/ElmKingdom/ElmKingdom". 비어 있으면 옛 세이브
FVector Location;
FRotator Rotation;   // Yaw만

// GameModes/ElmGameMode.h  (2단계)
virtual void RestartPlayer(AController* NewPlayer) override; // 이어하기 Transform이 있으면 RestartPlayerAtTransform

// UI/MainMenu/OptionsWidget.h  (3단계, WBP_Options의 부모)
// BindWidget: mWindowModeCombo / mResolutionCombo / mQualityCombo(UComboBoxString), mApplyButton, mBackButton
//             (4단계) mSensitivitySlider(USlider), mSensitivityText
void Open();   // 지금 설정을 읽어 콤보박스에 넣고 보인다
void Close();
// 창 모드 콤보 순서 = EWindowMode 값(0 전체 화면, 1 테두리 없는 창, 2 창). 품질 콤보 순서 = Scalability 단계(0 낮음 ~ 3 최고)

// Save/ElmGameUserSettings.h  (4단계, Project Settings → General Settings → Default Classes → Game User Settings Class)
UPROPERTY(Config) float mMouseSensitivity = 1.f;  // 배율
static UElmGameUserSettings* Get();               // 설정 전이면 nullptr
```

## 진행 상황

| 단계 | 내용 | 상태 |
|---|---|---|
| 1 | 메인 화면 골격 + 새 게임 + 게임 종료 (SaveSubsystem 페이드 공통화) | ☑ 2026-11-09 리뷰 통과. 남은 정리 하나: Game Default Map이 리다이렉터 `MainMenu_Change`를 가리킨다([에셋 작업](#에셋-작업)) |
| 2 | 이어하기: 세이브에 맵 / 위치, 불러오기, 그 자리에 스폰 | ◐ 2026-11-10 가이드 다시 전달(2026-11-09 판은 반영되지 않았다), 반영 전. Game Default Map 정리도 이 단계에서 함께 한다 |
| 3 | 옵션 창: 화면 모드 / 해상도 / 그래픽 품질 | ☐ |
| 4 | 마우스 감도: `UElmGameUserSettings`, `LookKey`, 옵션 창 슬라이더 | ☐ |
| 5 | 마무리: CLAUDE.md 갱신, 이 문서 정리 | ☐ |

### 1단계: 메인 화면 골격 + 새 게임 + 게임 종료
- 작업(코드):
  - 새 파일: `GameModes/MainMenuGameMode.h/.cpp`, `Player/MainMenuPlayerController.h/.cpp`, `UI/MainMenu/MainMenuWidget.h/.cpp`
  - `Subsystem/SaveSubsystem.h/.cpp`: `StartNewGame`, `MakeLevelURL`, `FadeOutAndOpen`을 더하고, `TravelToLevel`의 URL 만들기와 페이드를 이 두 함수로 옮긴다. 카메라 / 타이머를 캐릭터 대신 GameInstance로 찾는다(메인 화면에는 캐릭터가 없다).
  - `AMainMenuGameMode` 생성자: `DefaultPawnClass = nullptr`, `PlayerControllerClass = AMainMenuPlayerController`, `mNewGameLevel = /Game/Maps/ElmKingdom/KyleHouse.KyleHouse`, `mNewGameStartTag = "Room"`.
  - `AMainMenuPlayerController`: 생성자에서 `ConstructorHelpers::FClassFinder`로 `/Game/UI/MainMenu/WBP_MainMenu.WBP_MainMenu_C`를 찾는다. `BeginPlay`에서 `IsLocalController()`일 때만 위젯 생성 → `AddToViewport` → `FInputModeUIOnly`(`EMouseLockMode::DoNotLock`, 포커스 위젯 없음) → 커서 표시.
  - `UMainMenuWidget`(`UElmBaseWidget` 자식): `NativeOnInitialized`에서 새 게임 / 종료 버튼을 `AddDynamic`으로 연결하고, 이어하기 / 옵션 버튼은 2 / 3단계 전까지 `SetIsEnabled(false)`. 새 게임 = `GetAuthGameMode<AMainMenuGameMode>()->StartNewGame()`이 true면 `HideForTravel`. 종료 = `UKismetSystemLibrary::QuitGame(..., EQuitPreference::Quit, false)`.
- 작업(에디터): 아래 [에셋 작업](#에셋-작업)의 1단계 행.
- 순서: 코드 → 에디터 끄고 빌드 → 에디터 열기(주의 3의 에러 한 번) → `WBP_MainMenu` → MainMenu 맵 설정 → Game Default Map → **에디터 다시 열기** → PIE
- 테스트(MainMenu 맵을 열고 PIE):

| 확인 | 기대 결과 |
|---|---|
| PIE 시작 | 성과 Kyle 배경 위에 메뉴가 뜨고 커서가 보인다. HP / MP HUD가 없다. 이어하기 / 옵션 버튼은 흐리다. 메뉴가 안 뜨면 주의 3(에디터 재시작)을 의심한다 |
| Outliner(PIE 중) | `PlayerCharacter`가 없고 `MainMenuPlayerController`가 있다. 있으면 World Settings의 GameMode Override를 의심한다 |
| 새 게임 | 메뉴가 흐려지며 0.5초 동안 어두워지고, KyleHouse `Room` 자리에서 밝아진다. Lv1, 가방에 시작 아이템, 커서가 숨고 WASD / 마우스가 된다 |
| 새 게임 빠르게 두 번 | 한 번만 이동한다(`TravelToLevel :` 로그 한 줄) |
| 게임 종료 | PIE가 끝난다 |
| 회귀: 필드 맵(`ElmKingdom`)에서 PIE | 메인 화면 없이 예전처럼 새 게임으로 시작한다 |
| 회귀: 포탈 / B 귀환 / 사망 | 예전처럼 어두워졌다가 이동하고 밝아진다(`TravelToLevel`을 고쳤으므로) |
| 로그 | `LogTestDebug: TravelToLevel : /Game/Maps/ElmKingdom/KyleHouse#Room`. BindWidget 에러와 `LogHotReload`가 없다 |

### 2단계: 이어하기
- 작업: `ElmSaveGame.h`(필드 3개, `CurrentVersion = 2`), `PlayerCharacter.cpp`(`WriteToSave`에 맵 / 위치 / Yaw), `SaveSubsystem`(`HasSaveOnDisk`, `ContinueGame`(맵이 없으면 새 게임 자리), `ConsumeContinueTransform`, `OnPostLoadMap`에서 남은 이어하기 Transform 버리기, `WritePlayer`에서 버전 넣기, `SaveToDisk`에서 공중 거절), `ElmGameMode`(`RestartPlayer` 재정의), `MainMenuGameMode::ContinueGame`, `MainMenuWidget`(이어하기 버튼 켜기 / 연결).
- 테스트:

| 확인 | 기대 결과 |
|---|---|
| 세이브 없음(`Saved/SaveGames/ElmSave.sav` 지움) | 이어하기 버튼이 흐리다 |
| `ElmKingdom`에서 B → 저장 → PIE 종료 → MainMenu PIE → 이어하기 | `ElmKingdom`의 저장한 자리, 같은 방향으로 선다. 레벨 / HP / 경험치 / 골드 / 가방 / 장비 / 퀘스트가 같다 |
| 실내 맵(Tavern 등)에서 저장 → 이어하기 | 그 맵의 저장한 자리 |
| 이어하기 뒤 포탈 이동 | 포탈 도착 지점(태그)에 선다. 저장한 자리에 서면 주의 6 |
| 1단계 빌드로 만든 옛 세이브 → 이어하기 | KyleHouse `Room`에서 메모의 능력치 / 가방으로 시작 |
| 점프 중에 B → 저장하기 | "저장하지 못했습니다." 땅에 선 뒤에는 저장된다 |
| PIE 로그 | 이어하기로 연 URL에 `UEDPIE`가 없다 |

### 3단계: 옵션 창(화면 모드 / 해상도 / 그래픽 품질)
- 작업: `UI/MainMenu/OptionsWidget.h/.cpp`(새 파일), `MainMenuWidget`(`mOptionsWidget` BindWidget, 옵션 버튼), `WBP_Options`, `WBP_MainMenu`에 배치.
- 적용 순서: `SetFullscreenMode` → (테두리 없는 창이 아니면) `SetScreenResolution` → (골랐으면) `SetOverallScalabilityLevel` → `ConfirmVideoMode` → `ApplySettings(false)`(적용 + 저장). 해상도 목록은 `UKismetSystemLibrary::GetSupportedFullscreenResolutions`.
- 테스트: 창 모드 / 해상도는 **Standalone Game**(주의 7), 품질은 PIE에서도 보인다(주의 8).

| 확인 | 기대 결과 |
|---|---|
| 옵션 열기 | 반투명 배경 위에 옵션 창. 콤보박스에 지금 값이 골라져 있다. 뒤의 메뉴 버튼은 눌리지 않는다 |
| 테두리 없는 창 고르기 | 해상도 콤보박스가 흐려진다 |
| 창 + 1280×720 → 적용(Standalone) | 창이 그 크기가 된다. 게임을 다시 켜도 유지된다 |
| 품질 낮음 → 적용 | 그림자 / 화질이 눈에 띄게 낮아진다 |
| 뒤로(적용 없이) | 바꾼 값이 버려지고, 다시 열면 지금 값이 보인다 |

### 4단계: 마우스 감도
- 작업: `Save/ElmGameUserSettings.h/.cpp`(새 파일), Project Settings의 Game User Settings Class(에디터 재시작, 주의 11), `PlayerCharacter.cpp` `LookKey`(배율 곱하기), `OptionsWidget`(슬라이더 + 숫자).
- 테스트: 감도 0.5 / 2.0으로 적용 → 새 게임 → 카메라 회전 속도가 다르다. 게임을 다시 켜도 값이 남는다(GameUserSettings.ini에 `[/Script/ElmChronicles.ElmGameUserSettings]` 섹션).

### 5단계: 마무리
- CLAUDE.md: "메인 메뉴 레벨" 섹션(GameMode Override, PlayerStart 삭제, 컨트롤러 / 위젯), "레벨 이동 / 메모" 섹션(새 게임 / 이어하기 / 세이브 필드), 빌드 섹션의 시작 맵 줄(지금 "시작 맵은 `/Game/Maps/ElmKingdom/Field`"라고 적혀 있다 → `GameDefaultMap` = `MainMenu`, `EditorStartupMap` = `ElmKingdom`), 옵션(`UElmGameUserSettings`).
- 커밋은 사용자가 한다. 끝나면 이 문서를 지울지 사용자에게 묻는다.

## 에셋 작업

| 에셋 / 설정 | 할 일 | 담당 | 상태 |
|---|---|---|---|
| `Content/UI/MainMenu/WBP_MainMenu` (1단계) | 부모 `MainMenuWidget`, 빌드 뒤에 만든다. 루트 Canvas Panel(`RootCanvas`) 화면 전체. 그 아래 Vertical Box `MenuBox`(제목 Text "Elm Chronicles" 64 → Spacer 40 → 버튼 4개, 각 버튼 안 Text 28, Padding (40, 10)). `MenuBox` 슬롯은 오른쪽 가운데 앵커, Alignment (1, 0.5), Position (-120, 0), Size To Content. 버튼 4개 이름 = C++ 변수, **Is Focusable 끔**, 슬롯 Padding 아래 12 / Fill. 선택: 오른쪽 반투명 Image `RightShade`(Anchors (0.6, 0)~(1, 1), 색 (0, 0, 0, 0.35), `MenuBox`보다 먼저 그린다). 애니메이션 `FadeOut`: `RootCanvas`의 Render Opacity 0초 1 → 0.5초 0. (실제 루트 이름은 `CanvasPanel_31`) | 사용자 | ☑ 2026-11-09 (루트 Opacity 1, 버튼 4개 Is Focusable 끔 확인) |
| `Content/Maps/MainMenu` (1단계) | World Settings → GameMode Override = `MainMenuGameMode`. 쓰지 않는 `PlayerStart`(-11200, 0) 삭제 | 사용자 | ☑ 2026-11-09 |
| Project Settings → Maps & Modes (1단계) | Game Default Map = `MainMenu`(패키지 게임이 메인 화면에서 시작). Editor Startup Map은 필드 맵(`ElmKingdom`) | 사용자 | ◐ Game Default Map이 리다이렉터 `MainMenu_Change`를 가리킨다(주의 13). `MainMenu`로 다시 고른다 |
| `Content/UI/MainMenu/WBP_Options` (3단계) | 부모 `OptionsWidget`. 화면 전체 반투명 배경(Visible) + 가운데 창. 콤보박스 3개, 버튼 2개(Is Focusable 끔) | 사용자 | ☐ |
| `WBP_MainMenu` (3단계) | `WBP_Options`를 `mOptionsWidget`으로 배치(화면 전체 앵커, Hierarchy 맨 아래) | 사용자 | ☐ |
| Project Settings → General Settings → Default Classes (4단계) | Game User Settings Class = `ElmGameUserSettings` → 에디터 재시작 | 사용자 | ☐ |
| `WBP_Options` (4단계) | `mSensitivitySlider`(Slider), `mSensitivityText`(TextBlock) 추가 | 사용자 | ☐ |

## 진행 방식
- **Claude는 C++ 소스와 에셋을 직접 고치지 않는다.** 단계마다 따라 하는 가이드(파일:위치 → 바꾸기 전 / 후 코드 → 빌드 → 에디터 작업 → PIE 확인, 단계마다 짧은 "왜")를 주고, 사용자가 입력한다.
- 사용자가 "N단계 완료"라고 하면 Claude가 확인한다: `git diff`, `Binaries/Win64/UnrealEditor-ElmChronicles*.dll` 시간과 `-000N.dll` 여부, `Saved/Logs/ElmChronicles.log`의 `LogHotReload` / `LogTestDebug` / BindWidget 에러.
- 새 소스 파일은 UTF-8(BOM)로 만든다. 기존 파일(예: `MenuWidget.h/.cpp`)을 탐색기에서 복사해 이름을 바꾸면 BOM이 유지된다. 한글 `NSLOCTEXT`가 들어가는 파일은 특히 확인한다(`head -c3 파일 | od -An -tx1` = `ef bb bf`).
- 빌드는 에디터를 끄고 CLAUDE.md의 `Build.bat` 명령으로 한다.
- MD 문서(CLAUDE.md, Docs)는 Claude가 묻지 않고 고친다.
- 2026-11-08 시작 시점에 다른 작업의 미커밋 변경이 작업 트리에 있다. 리뷰할 때 이 파일들은 메인 화면 작업과 구분한다.
  - 캐릭터 창(`Docs/CharacterWindow_Progress.md` 3단계 "색 맞추기"): `CharacterPreviewActor.h/.cpp`, `M_CharacterPreview`, `RT_CharacterPreviewMask`
  - Field 하늘 섬: `Field.umap`, `Field_Backup.umap`, `Docs/Plan.md`, `CLAUDE.md`
  - 그 밖의 KayKit 메시 에셋(충돌 설정 변경)

## 이후 작업 (이 계획의 범위 밖)
- B 메뉴에 "메인 화면으로" / "옵션" 버튼(2026-11-08 이번에는 넣지 않기로 했다). 옵션 창은 메인 화면에 의존하지 않게 만들어 두므로 `WBP_Menu`나 `WBP_PlayerHUD`에 놓고 일시정지 / 입력 모드만 맞추면 된다.
- 소리 볼륨 옵션(소리 에셋과 SoundClass / SoundMix가 생기면).
- 세이브 슬롯 여러 개, 세이브 정보(저장한 맵 / 레벨 / 시간) 표시.
- 메인 화면 BGM, 시작할 때 페이드 인.
