# 메뉴 UI (B 키)

> **다음 세션 시작 방법**: 이 문서를 읽고 → `git status` / `git diff`로 어디까지 했는지 확인하고 → [진행 상황](#진행-상황) 표에서 첫 번째 ☐ 단계부터 이어서 한다. 코드와 에셋은 사용자가 입력한다([진행 방식](#진행-방식)).
>
> 마지막 갱신: 2026-10-30

## 목표

게임 중 B를 누르면 화면이 어두워지고 게임이 멈춘 채로 메뉴 창이 뜬다. 메뉴에는 **계속하기 / 귀환 / 저장하기 / 게임 종료**가 있다. 귀환은 어느 맵에서나 KyleHouse의 `PlayerStart_House`(태그 `Room`)로 보내고, 전투 중이거나 죽어 있으면 막힌다. 저장하기는 메모(`UElmSaveGame`)를 디스크에 쓴다.

## 배경

- [Plan.md](Plan.md) "2. 조작"에 `메뉴 | B`만 있고 내용은 정해져 있지 않았다. 인벤토리(I) / 퀘스트(P) / 지도(M)는 따로 키가 있으므로 메뉴에 넣지 않았다.
- [LevelTravel_Progress.md](LevelTravel_Progress.md)에서 KyleHouse에 귀환용 PlayerStart(`PlayerStart_House`, 태그 `Room`)를 미리 배치해 두었다. 화면 전환, 메모 적기, 중복 이동 방지는 `USaveSubsystem::TravelToLevel`이 이미 한다.
- 지금 HUD의 창(인벤토리, 대화)은 모두 `WBP_PlayerHUD` 안의 자식 위젯이고 `BindWidget`으로 연결되어 있다. 창이 열렸는지는 `UPlayerHUDWidget::IsAnyWindowOpen`이 모으고, 커서 / 입력 모드는 `AElmPlayerController::RefreshInputMode`가 맞춘다.

## 결정 사항

| 항목 | 결정 | 이유 / 버린 안 |
| --- | --- | --- |
| 메뉴 항목 | 계속하기, 귀환, 저장하기, 게임 종료 (2026-10-29 사용자 선택) | 버린 안: 다른 창 바로가기(인벤토리 / 퀘스트 / 지도). 각자 키가 있다 |
| 이름 | 버튼 글자는 **"귀환"**(처음 안 "집으로 귀환"에서 사용자가 바꿈). 코드 이름은 `Recall`(`mRecallButton`, `AElmGameModeBase::Recall`) | |
| 열려 있는 동안 | 일시정지. `APlayerController::SetPause(true)` | 싱글플레이 액션 RPG(젤다, 위처)에서 흔한 방식. 버린 안: 인벤토리처럼 게임 계속 진행(전투 중 메뉴를 열면 맞는다) |
| 메뉴 위젯 위치 | `WBP_PlayerHUD`의 자식 `mMenuWidget`(BindWidget). 화면 전체를 채우고 맨 위에 그린다 | 인벤토리 / 대화창과 같은 방식이라 `IsAnyWindowOpen` / `RefreshInputMode`를 그대로 쓴다. 버린 안: 따로 `AddToViewport` |
| 위젯 배치 | 사용자가 WBP 디자이너에서 만든다 | unreal-mcp에 위젯 트리(디자이너) 편집 도구가 없다(2026-10-29 확인). 버린 안: C++에서 위젯 트리를 코드로 생성(다른 창과 방식이 달라지고 모양을 바꿀 때마다 코드를 고쳐야 함) |
| 버튼이 하는 일 | 위젯은 일을 맡은 곳에 부탁하고 결과 문구만 띄운다. 귀환 → `AElmGameModeBase::Recall`, 저장 → `USaveSubsystem::SaveToDisk`, 닫기 → `AElmPlayerController::SetMenuOpen(false)` | 사망 후 병원 이동이 GameMode에 있는 것과 같다. "어디로 귀환하나 / 언제 막나"는 게임 규칙이다 |
| 귀환 위치 | `AElmGameModeBase` 생성자에서 `mRecallLevel` = `/Game/Maps/ElmKingdom/KyleHouse.KyleHouse`, `mRecallStartTag` = `Room` | `mRespawnLevel` / `mRespawnStartTag`와 같은 방식 |
| 전투 중 판정 | 맵의 `AElmAIController` 중 **살아 있는 Pawn을 조종하고 Blackboard `Target`이 이 플레이어**인 것이 하나라도 있으면 전투 중 (`AElmGameModeBase::IsInCombat`) | `ANPCCharacter::CanInteract`(싸우는 중엔 대화 금지)와 같은 기준이다. 버린 안: "최근 N초 안에 맞았으면 전투 중"(타이머와 상태가 새로 필요) |
| 귀환을 막는 경우 | 전투 중, 플레이어 사망. 메뉴를 열 때 `mRecallButton`을 끄고, 전투 중이면 "전투 중에는 귀환할 수 없습니다." 문구 | 죽은 채로 귀환하면 메모 HP 0 → KyleHouse에서 HP가 가득 찬 채 시작(병원 대신 집에서 부활)하게 된다 |
| 귀환 순서 | 메뉴 닫기(일시정지 해제) → `Recall` | [주의](#주의)의 "일시정지 중 TravelToLevel" |
| 저장 | `USaveSubsystem::SaveToDisk(Player)` = `WritePlayer` + `UGameplayStatics::SaveGameToSlot(mSaveGame, "ElmSave", 0)`. 결과로 "저장했습니다." / "저장하지 못했습니다." 문구 | 포탈 이동과 같은 메모를 쓴다. 메모는 새 레벨이 초기화할 때만 읽으므로 지금 덮어써도 이번 레벨에는 영향이 없다 |
| 저장을 막는 경우 | 플레이어 사망 | HP 0을 저장하지 않는다 |
| 불러오기 | 이번 범위 밖 | 타이틀 / 이어하기 화면이 없다. [이후 작업](#이후-작업-이-계획의-범위-밖) |
| 게임 종료 | 같은 창 안의 확인 상자(`mQuitConfirmBox`) → "종료" → `UKismetSystemLibrary::QuitGame` | 버린 안: 확인 창용 위젯을 따로 만들기(지금은 쓰는 곳이 하나뿐) |
| 죽은 상태에서 메뉴 | 열 수 있다(게임 종료는 가능). 열려 있는 동안 병원 이동 타이머(`RespawnDelay`)도 멈췄다가 닫으면 이어서 흐른다 | 따로 막을 이유가 없다 |

## 주의

조사하면서 확인한 함정. 아직 코드가 없으므로 "증상"은 이렇게 하지 않았을 때 생기는 일이다.

| 증상 | 원인 | 해결 |
| --- | --- | --- |
| B로 메뉴는 열리는데 다시 B를 눌러도 안 닫힘 | 일시정지 중에는 Enhanced Input이 `bTriggerWhenPaused`가 꺼진 액션을 발동하지 않는다 | `IA_Menu`만 **Trigger when Paused** 체크. 다른 IA는 그대로 두어서 메뉴가 열린 동안 이동 / 공격 / I / E가 막히게 한다 |
| 메뉴가 열린 동안 B가 안 먹음 | `FInputModeUIOnly`로 바꾸면 키 입력이 Slate 위젯으로만 가고 PlayerController에 오지 않는다 | `RefreshInputMode`의 `FInputModeGameAndUI`를 그대로 쓴다 |
| 버튼을 클릭한 뒤 Space / Enter를 누르면 그 버튼이 또 눌림 | 클릭한 버튼이 키보드 포커스를 가져가고, 버튼은 Space / Enter를 클릭으로 처리한다 | WBP에서 모든 버튼의 **Is Focusable** 체크 해제 |
| 귀환을 눌렀는데 화면만 멈추고 이동하지 않음, 이후 포탈도 안 됨 | 일시정지 중에는 월드 타이머가 돌지 않는다. `TravelToLevel`은 `mIsTraveling = true` 후 페이드 타이머로 `OpenLevel`을 부르므로, 멈춘 채로 부르면 타이머가 영영 안 돌고 `mIsTraveling`이 true로 남는다 | 메뉴를 먼저 닫고(`SetPause(false)`) `Recall`을 부른다 |
| 쓰러진 몬스터 / 필수 NPC 근처에서 귀환이 계속 막힘 | AI가 죽으면 `AAICharacter::OnKilledBy`가 BT를 `StopLogic`만 하고 Blackboard `Target`은 지우지 않는다. 몬스터는 사망 애니메이션 동안, 필수 NPC는 쓰러져 있는 동안 Target이 남는다 | `IsInCombat`에서 `AElmAIController::IsDeadTarget(It->GetPawn())`인 컨트롤러는 건너뛴다 |
| 아이템 정보창이 메뉴 뒤(어두운 배경 아래)에 남음 | 아이콘 위에 마우스를 올린 채 메뉴를 열면 MouseLeave가 오지 않는다(`SetInventoryVisible`의 주석과 같은 이유) | 메뉴를 열 때 `EnableItemInfoWidget(false)` |
| PIE 시작 직후 크래시(`mMenuWidget` null) | C++에 `BindWidget`을 추가했는데 `WBP_PlayerHUD`에 같은 이름의 위젯이 없으면 WBP 컴파일 에러가 나고 변수가 비어 있다 | 1단계 빌드 후 PIE 전에 `WBP_PlayerHUD`에 `mMenuWidget`을 넣고 컴파일 |
| `DefaultInput`에서 `IA_Menu`를 못 찾음 | `UDefaultInput` CDO는 에디터 시작 때 한 번 만들어진다. 그때 `IA_Menu` 파일이 없으면 다시 시작할 때까지 `"Menu"`가 없다 | `IA_Menu`를 먼저 만들어 저장한 뒤에 새 코드로 에디터를 연다 |
| 새 `UCLASS`를 에디터를 켠 채 빌드(Hot Reload)한 세션에서 `WBP_PlayerHUD_C is not a child class of Class /Script/ElmChronicles.PlayerHUDWidget`, `CDO Constructor (ElmPlayerController): Failed to find .../WBP_PlayerHUD_C` | Hot Reload가 `UPlayerHUDWidget`을 새 클래스로 바꿨는데 `WBP_PlayerHUD`는 옛 클래스를 부모로 들고 있었다. 컨트롤러 CDO가 그 순간 WBP 클래스를 못 찾아 `mPlayerHUDWidgetClass`가 비고, 그 세션의 PIE는 HUD 없이 뜰 수 있다. 2026-10-30 00:22 1단계 빌드에서 실제로 났다(`Saved/Logs/ElmChronicles.log`, `UnrealEditor-ElmChronicles-0003.dll`, `UnrealEditor.modules`가 `-0003`을 가리킴) | 에디터를 끄고 `Build.bat` → 에디터를 다시 열고 테스트한다. Hot Reload 세션에서 한 PIE 결과는 믿지 않는다 |

## 목표 구조

```
AElmPlayerController (Player/)
   SetupInputComponent        "Menu"(IA_Menu, B) → ToggleMenu
   ToggleMenu()               SetMenuOpen(!IsMenuVisible)
   SetMenuOpen(bool Open)     HUD->SetMenuVisible(Open) → SetPause(Open) → RefreshInputMode

UPlayerHUDWidget (UI/Game/)
   mMenuWidget (BindWidget, UMenuWidget)
   SetMenuVisible(bool)       열 때 mMenuWidget->Refresh() + 정보창 닫기
   IsMenuVisible() const
   IsAnyWindowOpen()          인벤토리 || 메뉴
   NativeOnInitialized        SetMenuVisible(false)

UMenuWidget : UElmBaseWidget (UI/Game/MenuWidget.h, 새 파일)
   BindWidget: mMainBox, mResumeButton, mRecallButton, mSaveButton, mQuitButton,
               mQuitConfirmBox, mQuitYesButton, mQuitNoButton, mMessageText
   Refresh()                  문구 지우기, 확인 상자 닫기, 귀환 / 저장 버튼 켜고 끄기
   UFUNCTION On...Clicked     버튼마다 하나 (OnClicked는 Dynamic 델리게이트라 UFUNCTION이어야 AddDynamic 가능)
   SetMessage(FText) / ShowQuitConfirm(bool)

AElmGameModeBase (GameModes/)
   mRecallLevel / mRecallStartTag            KyleHouse / "Room"
   bool IsInCombat(const APlayerCharacter*) const
   bool CanRecall(const APlayerCharacter*) const   살아 있고 전투 중이 아님
   void Recall(APlayerCharacter*)                  CanRecall 재확인 → TravelToLevel(Player, mRecallLevel, mRecallStartTag)

USaveSubsystem (Subsystem/)
   static constexpr const TCHAR* SaveSlotName = TEXT("ElmSave")
   bool SaveToDisk(APlayerCharacter*)        mIsTraveling이면 false → WritePlayer → SaveGameToSlot
```

## 진행 상황

| 단계 | 내용 | 상태 |
| --- | --- | --- |
| 1 | `IA_Menu`(B) + 메뉴 열기 / 닫기 + 일시정지 + 계속하기 | ◐ 코드·에셋 완료, 리뷰 통과(2026-10-30). 남은 것: 에디터를 끈 상태에서 `Build.bat` → 다시 열고 테스트 표 확인 |
| 2 | 게임 종료(확인 상자) | ☐ |
| 3 | 귀환(전투 중 / 사망 시 막기) | ☐ |
| 4 | 저장하기 | ☐ |

### 1단계: 메뉴 열기 / 닫기 + 일시정지 + 계속하기

- 작업
  - 에셋: `IA_Menu`, `IMC_Default`([에셋 작업](#에셋-작업))
  - `DefaultInput.cpp`: `"Menu"` 등록
  - 새 파일 `UI/Game/MenuWidget.h/.cpp`: BindWidget 전부, `Refresh`, `OnResumeClicked`, `SetMessage`, `ShowQuitConfirm`
  - `PlayerHUDWidget.h/.cpp`: `mMenuWidget`, `SetMenuVisible`, `IsMenuVisible`, `IsAnyWindowOpen`, `NativeOnInitialized`
  - `ElmPlayerController.h/.cpp`: `ToggleMenu`, `SetMenuOpen`, `"Menu"` 바인딩
  - 에셋: `WBP_Menu` 새로 만들기, `WBP_PlayerHUD`에 `mMenuWidget` 넣기
- 빌드: 새 `UCLASS`와 헤더 변경이 있으므로 에디터를 끄고 `Build.bat`
- 테스트

| 확인 | 기대 결과 |
| --- | --- |
| B | 화면이 어두워지고 가운데에 메뉴. 커서가 보인다. 몬스터 / NPC / 애니메이션이 멈춘다. "게임을 종료할까요?" 상자와 안내 문구는 보이지 않는다 |
| 메뉴가 열린 채 WASD, 마우스 이동, I, E, Space | 아무 일도 없다 |
| B 다시 | 메뉴가 닫히고 커서가 사라지며 게임이 다시 움직인다 |
| 계속하기 클릭 | B와 같다 |
| 계속하기 클릭 후 Space | 점프만 한다(메뉴가 다시 열리지 않는다) |
| I로 인벤토리를 연 채 B → B | 메뉴가 닫혀도 인벤토리와 커서는 남아 있다 |
| 귀환 / 저장하기 / 게임 종료 클릭 | 아직 아무 일도 없다(2 ~ 4단계) |

실패하면: B로 안 닫힘 → `IA_Menu`의 Trigger when Paused. B로 안 열림 → `IMC_Default`의 B 매핑, `DefaultInput` 경로, `IA_Menu`를 저장한 뒤 에디터를 다시 열었는지. PIE 시작 크래시 → `WBP_PlayerHUD`에 `mMenuWidget` 이름. HUD가 통째로 안 보임 → [주의](#주의)의 Hot Reload 항목.

2026-10-30 리뷰 결과:
- 코드(`DefaultInput.cpp`, `MenuWidget.h/.cpp`, `PlayerHUDWidget.h/.cpp`, `ElmPlayerController.h/.cpp`)가 가이드와 같다.
- 에셋: 아래 [에셋 작업](#에셋-작업) 표 참고. `WBP_Menu` 마지막 컴파일(00:36)과 `WBP_PlayerHUD` 마지막 컴파일(00:41)에 바인딩 에러가 없다.
- 빌드가 Hot Reload(`-0003.dll`)로 들어갔다. 그 세션에서 PIE를 두 번(각 10초 정도) 했지만 위 주의 때문에 결과를 믿을 수 없어서 테스트 표는 아직 확인하지 않은 것으로 둔다.
- 버튼 6개의 Is Focusable 해제는 파일로 개수를 셀 수 없다(이름표에 `IsFocusable`이 한 번만 저장된다). 테스트 표의 "계속하기 클릭 후 Space"로 확인한다.

### 2단계: 게임 종료

- 작업: `MenuWidget`에 `OnQuitClicked`(확인 상자 열기) / `OnQuitNoClicked`(닫기) / `OnQuitYesClicked`(`UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false)`, `#include "Kismet/KismetSystemLibrary.h"`) 추가, `NativeOnInitialized`에서 연결
- 빌드: 헤더에 `UFUNCTION` 추가 → 에디터를 끄고 빌드
- 테스트

| 확인 | 기대 결과 |
| --- | --- |
| 게임 종료 클릭 | 버튼 목록이 사라지고 "게임을 종료할까요?" + 종료 / 취소 |
| 취소 | 버튼 목록으로 돌아온다 |
| 확인 상자를 연 채 B로 닫고 다시 B | 버튼 목록이 보인다(`Refresh`가 확인 상자를 닫음) |
| 종료 | PIE가 끝난다(에디터는 그대로) |

### 3단계: 귀환

- 작업
  - `ElmGameModeBase.h/.cpp`: `mRecallLevel` / `mRecallStartTag`(생성자), `IsInCombat`, `CanRecall`, `Recall`. `#include "../AI/ElmAIController.h"`
  - `MenuWidget`: `OnRecallClicked`(메뉴 닫기 → `Recall`), `Refresh`에서 `mRecallButton->SetIsEnabled(CanRecall)` + 전투 중 문구
- 빌드: 헤더 변경 → 에디터를 끄고 빌드
- 테스트

| 확인 | 기대 결과 |
| --- | --- |
| Field(몬스터 없는 곳)에서 B → 귀환 | 메뉴가 닫히고 페이드 → KyleHouse `PlayerStart_House`에 선다. Output Log `TravelToLevel : .../KyleHouse#Room` 한 번 |
| 귀환 전에 HP / MP / Gold / 가방을 바꿔 둠 | KyleHouse에서 그대로 |
| 몬스터가 쫓아오는 중에 B | 귀환 버튼이 꺼져 있고 "전투 중에는 귀환할 수 없습니다." |
| 몬스터를 처치한 직후(사망 애니메이션 중) B | 다른 적이 없으면 귀환 버튼이 켜져 있다 |
| 몬스터 시야 밖으로 도망친 뒤 B | 귀환 버튼이 켜져 있다(Target을 놓친 뒤) |
| 죽은 직후 B | 귀환 버튼이 꺼져 있다. 메뉴를 닫으면 병원으로 이동한다 |
| KyleHouse 안에서 귀환 | 같은 자리로 다시 이동(막지 않는다) |

### 4단계: 저장하기

- 작업
  - `SaveSubsystem.h/.cpp`: `SaveSlotName`, `SaveToDisk`
  - `MenuWidget`: `OnSaveClicked`(결과 문구), `Refresh`에서 죽었으면 `mSaveButton` 끄기
- 빌드: 헤더 변경 → 에디터를 끄고 빌드
- 테스트

| 확인 | 기대 결과 |
| --- | --- |
| 저장하기 클릭 | "저장했습니다." 문구. `Saved/SaveGames/ElmSave.sav`가 생기거나 수정 시간이 바뀐다 |
| 메뉴를 닫았다 다시 열기 | 문구가 지워져 있다 |
| 저장 후 포탈 이동 | 평소처럼 이동, 가방 / 능력치 유지(메모를 덮어쓴 영향 없음) |
| 죽은 직후 B | 저장하기 버튼이 꺼져 있다 |

## 에셋 작업

| 에셋 | 할 일 | 담당 | 상태 |
| --- | --- | --- | --- |
| `Content/Input/IA_Menu` | Input Action. Value Type `Digital (bool)`, **Trigger when Paused 체크** | 사용자 | ☑ 2026-10-29 저장(미커밋). 파일에 `bTriggerWhenPaused`, `Boolean`이 있다 |
| `Content/Input/IMC_Default` | `IA_Menu` ← 키 `B` 매핑 추가 | 사용자 | ☑ 파일에 `IA_Menu` 참조가 있다(미커밋) |
| `Content/UI/Game/WBP_Menu` | 새 Widget Blueprint, 부모 `MenuWidget`. 위젯 트리는 1단계 안내. 버튼 6개 모두 Is Focusable 해제 | 사용자 | ☑ 2026-10-30. `m`으로 시작하는 9개 이름 모두 있다. Is Focusable은 테스트로 확인 |
| `Content/UI/Game/WBP_PlayerHUD` | Canvas의 **마지막 자식**으로 `WBP_Menu` 추가, 이름 `mMenuWidget`, 앵커 화면 전체 + Offset 0 | 사용자 | ☑ 2026-10-30. `mMenuWidget`, `WBP_Menu` 참조가 있다 |

## 진행 방식

- 코드와 에셋은 사용자가 직접 입력한다. Claude는 단계마다 그대로 입력할 수 있는 전체 코드, 정확한 위치(파일, 함수), WBP 위젯 트리, 테스트 표, 초보자용 설명을 준다. 2026-10-29 Claude가 unreal-mcp로 에셋을 만들기 시작했다가 사용자가 중단하고 "그냥 제가 할게요. 가이드만 해주세요"라고 했다. Claude는 사용자가 "직접 해 줘"라고 할 때만 코드나 에셋을 고친다.
- 루프: 단계 안내 → 사용자가 입력, 빌드, PIE 테스트 → "N단계 완료" → Claude가 `git diff`와 파일로 리뷰 → 통과하면 이 문서의 상태를 ☑로 바꾸고 다음 단계.
- 이 문서와 CLAUDE.md의 갱신은 사용자에게 묻지 않고 Claude가 한다.
- 헤더나 `UPROPERTY` / `UCLASS` / `UFUNCTION`이 바뀌는 단계는 "에디터를 끄고 빌드"를 안내한다(1 ~ 4단계 모두 해당).
- 코드 주석은 한글 "~한다" 체로, "왜 이렇게 하는지"를 적는다.

## 이후 작업 (이 계획의 범위 밖)

- 불러오기 / 이어하기(타이틀 화면). 지금 저장 파일에는 맵과 위치가 없다. `UElmSaveGame`에 맵 경로와 위치(또는 PlayerStart 태그)를 추가하고 `SaveVersion`을 올려야 한다. PIE에서는 맵 이름에 `UEDPIE_0_` 접두사가 붙으므로 저장할 때 뗀다.
- 끝나면 CLAUDE.md "입력 / 상호작용 / UI"에 메뉴(일시정지, `IA_Menu`의 Trigger when Paused, 버튼 Is Focusable 해제)를 옮긴다.
