# 레벨 이동

> **다음 세션 시작 방법**: 이 문서를 읽고 → `git status` / `git diff`로 어디까지 했는지 확인하고 → [진행 상황](#진행-상황) 표에서 첫 번째 ☐ / ◐ 단계부터 이어서 한다.
>
> 마지막 갱신: 2026-10-28

## 목표

포탈(문, 동굴 입구) 앞에서 E를 누르면 화면이 어두워지고 다른 맵으로 옮겨 가서, 정해 둔 PlayerStart에 선 채로 다시 밝아진다. 가방(칸 위치 포함)과 능력치(Level / HP / MP / Exp / Gold)는 이동 전 그대로 유지된다. 선택 단계로, 플레이어가 죽으면 병원 맵에서 다시 시작한다(Plan.md "사망 시 마을 병원에서 다시 시작").

## 배경

- `OpenLevel`(non-seamless travel)은 World, PlayerController, PlayerState(ASC + AttributeSet), Pawn(`UInventoryComponent`)을 모두 버리고 새로 만든다. 남는 것은 GameInstance와 그 서브시스템뿐이다. 그래서 이동 직전에 상태를 서브시스템이 들고 있는 "메모"(`UElmSaveGame`)에 적고, 새 레벨의 부품들이 초기화할 때 그 메모를 읽는다.
- 작업 전에는 `Test`와 `Field` 맵만 World Settings에 `ElmGameModeBase`가 지정돼 있었다. 나머지 맵(Hospital, KyleHouse, OrcCave, Palace, Shop, Tavern, VoidRaindungeon)은 엔진 기본 GameMode로 열려서 플레이어 캐릭터가 나오지 않았다.
- 맵 `Content/Maps/ElmKingdom/`의 대부분은 PlayerStart가 2개다. 엔진 기본 `ChoosePlayerStart`는 빈 자리 중에서 무작위로 고른다.

## 결정 사항

| 항목 | 결정 | 이유 / 버린 안 |
| --- | --- | --- |
| GameMode 지정 | `DefaultEngine.ini`의 `GlobalDefaultGameMode=/Script/ElmChronicles.ElmGameModeBase` | 맵마다 World Settings를 고치지 않아도 된다. 맵에 override가 있으면 그것이 우선한다 |
| 도착 위치 | URL `"맵경로#태그"`. `#` 뒤 값을 엔진이 Portal 옵션으로 보고 `FindPlayerStart`가 그 `PlayerStartTag`를 가진 PlayerStart를 먼저 찾는다 | 엔진 기본 흐름(Login → `InitNewPlayer` → `FindPlayerStart(Player, Portal)`)이라 코드가 필요 없다 |
| 태그 규칙 | 도착 PlayerStart의 Player Start Tag = **출발 맵 이름**. Field → 동굴은 `OrcCave#Field`, 동굴 → Field는 `Field#OrcCave` | 지금 배치한 두 맵이 이 규칙을 따른다(Output Log `TravelToLevel :`로 확인) |
| 태그 없이 들어왔을 때 | `AElmGameModeBase::ChoosePlayerStart_Implementation`: `APlayerStartPIE`(뷰포트 "Play From Here")가 있으면 그것, 없으면 태그 없는 첫 PlayerStart, 그것도 없으면 엔진 기본 | 엔진 기본은 무작위라 PIE를 시작할 때마다 서는 자리가 바뀌었다. 사용자가 직접 추가했다 |
| 메모 보관 | `USaveSubsystem`(GameInstanceSubsystem)이 `UPROPERTY() TObjectPtr<UElmSaveGame> mSaveGame`을 든다. nullptr = 새 게임 | 나중에 디스크 저장도 같은 객체를 `SaveGameToSlot`에 넘기면 된다 |
| 쓰기 / 읽기 위치 | 쓰기는 한 곳(`TravelToLevel` → `WritePlayer` → `APlayerCharacter::WriteToSave`). 읽기는 부품마다 자기 초기화 시점(`UInventoryComponent::InitInventory`, `APlayerCharacter::InitAttributes`) | 초기화 함수가 이미 `CallOrRegisterDataLoaded`를 거치므로 테이블이 준비된 뒤에 읽는다. 새 레벨에서는 데이터가 이미 로드돼 있어 즉시 불린다 |
| 쓰는 시점 | E를 누른 순간(페이드 아웃 시작 전) | "문을 나선 순간"의 상태를 기준으로 한다. 어두워지는 0.5초 동안 생긴 변화는 적지 않는다 |
| 능력치 범위 | Level, HP, MP, Exp, Gold만 적는다 | MaxHP, 공격력 등은 Level로 `DT_LevelStat`에서 다시 계산된다 |
| 능력치 읽기 / 쓰기 | `GetNumericAttributeBase` / `SetNumericAttributeBase` (Base 값) | 지속형 GE의 버프(장비 등)가 저장값에 섞이지 않는다 |
| 화면 전환 담당 | `USaveSubsystem`이 페이드 아웃(`TravelToLevel`)과 페이드 인(`OnPostLoadMap` → `FadeIn`)을 모두 맡는다 | 버린 안: PlayerController / 카메라. 레벨과 함께 사라져서 "아까 어둡게 했다"는 사실을 도착 레벨로 넘길 수 없다 |
| 도착 알림 | `FCoreUObjectDelegates::PostLoadMapWithWorld`. `Initialize`에서 등록, `Deinitialize`에서 `RemoveAll` | 엔진 `UEngine::LoadMap`이 플레이어 스폰 → `World->BeginPlay()` → 이 알림 순서라서, 알림 시점에 새 컨트롤러와 카메라가 이미 있다. 전역 알림이라 `World->GetGameInstance() != GetGameInstance()`인 월드는 거른다 |
| 연타 / 중복 이동 방지 | 두 겹. 입력 끄기(`Player->DisableInput(PC)` + `PC->DisableInput(PC)`)와 `mIsTraveling` | 입력을 끄면 E가 안 들어오고, `mIsTraveling`은 키가 아닌 경로(Step 5의 사망 이동 타이머 등)까지 막는다. 새 레벨에서는 Pawn과 컨트롤러가 새로 만들어지므로 입력을 다시 켤 필요가 없다 |
| 페이드 시간 | 페이드 아웃 0.5초 → `OpenLevel` → 도착 후 검은 화면 유지 0.5초(`ArriveHoldTime`) → 페이드 인 0.5초 | 유지 시간이 필요한 이유는 [주의](#주의)의 "도착 직후 화면이 번쩍 켜짐" |

## 주의

| 증상 | 원인 | 해결 |
| --- | --- | --- |
| Grep(rg)으로 `.umap`에서 `GameMode`를 찾으면 아무것도 안 나와서 "참조 없음"으로 오판 | rg는 바이너리 파일을 건너뛴다 | `grep -aoE "<문자열>" <맵.umap>` 처럼 `-a`로 바이너리를 텍스트로 읽는다 |
| PIE에서 Output Log에 `TravelToLevel : /Game/Maps/ElmKingdom/UEDPIE_0_OrcCave#Field` | PIE가 이미 로드한 맵을 가리키는 Soft 경로에 `FSoftObjectPath::FixupForPIE`가 `UEDPIE_0_` 접두사를 붙인다 | 정상이다. `LoadMap`이 접두사를 떼고 정상 맵을 연다. 패키징 빌드에서는 붙지 않는다 |
| 이동 후 HP / MP가 0이 됨(가능성) | `PreAttributeBaseChange`가 HP / MP를 MaxHP / MaxMP로 Clamp한다. Max가 0일 때 HP를 넣으면 0으로 잘린다 | `InitAttributes`에서 `InitLevelStats`(Max를 넣고 HP / MP를 가득 채움) 다음에 메모 값을 덮는다. 순서를 바꾸지 않는다 |
| 도착 직후 화면이 한 프레임 만에 확 밝아짐 | 로딩이 끝난 다음 프레임의 DeltaTime에 로딩 시간이 한꺼번에 들어온다. `WorldSettings`의 `MaxUndilatedFrameTime`(BaseGame.ini 0.4초)까지 잘리지만, 이 프로젝트의 `LoadMap`은 0.25 ~ 0.59초가 걸려서(`Saved/Logs`의 `Took ... seconds to LoadMap`) 0.5초 페이드가 한 프레임에 50 ~ 80% 진행된다 | 도착하면 `SetManualCameraFade(1)`로 검정을 고정하고 `ArriveHoldTime`(0.5초) 뒤에 페이드 인을 시작한다 |
| 페이드 아웃이 끝나고 `OpenLevel` 전에 화면이 번쩍 | `StartCameraFade`의 `bHoldWhenFinished`가 false면 끝나는 순간 `StopCameraFade`로 페이드가 꺼진다 | 페이드 아웃은 6번째 인자 `true`(검정 유지) |
| `ArriveHoldTime`을 0으로 하면 검은 화면에서 멈춤 | 시간이 0 이하인 타이머는 걸리지 않아 `FadeIn`이 불리지 않는다 | 0보다 큰 값을 쓴다 |
| `FTimerDelegate::CreateUObject(this, &F, URL)` 컴파일 에러 | Payload를 받는 매개변수가 참조(`const FString&`)면 안 된다 | `OpenLevelURL(FString URL)`처럼 값으로 받는다(`AssetSubsystem::OnGameDataLoaded`와 같다) |
| 캐릭터 입력만 끄면 E / I가 계속 동작 | `APlayerController::BuildInputStack`이 Pawn 입력과 컨트롤러 입력의 켜짐 상태를 따로 본다 | 둘 다 `DisableInput` |
| 검은 화면 위에 HP 바, `[E] 이동하기` 문구가 보임 | 카메라 페이드는 3D 화면에 색을 덧씌우는 방식(`ViewInitOptions.OverlayColor`)이라 UMG는 덮지 않는다 | 지금은 그대로 둔다. 가리려면 화면 전체를 덮는 로딩 위젯으로 바꾼다. `TravelToLevel`과 `FadeIn` 두 곳만 바꾸면 된다 |
| 에디터를 켠 채 IDE / UBT로 빌드하면 `UnrealEditor-ElmChronicles-000N.dll`이 생기고 Hot Reload를 시도 | Live Coding이 시작되지 않은 에디터에서는 UBT가 빌드를 막지 않는다. 2026-10-28 현재 `Binaries/Win64`에 `-0001` ~ `-0009` DLL이 있다 | 헤더를 바꾼 단계는 에디터를 끄고 `Build.bat`으로 빌드한다(CLAUDE.md "빌드 / 실행") |

## 목표 구조

```
ALevelPortal (Interaction/)            IInteractable. mTargetLevel(TSoftObjectPtr<UWorld>), mTargetStartTag, mInteractText
   │ Interact → GetGameInstance()->GetSubsystem<USaveSubsystem>()->TravelToLevel(...)
   ▼
USaveSubsystem (Subsystem/)            GameInstanceSubsystem. mSaveGame, mIsTraveling, FadeTime / ArriveHoldTime
   TravelToLevel(Player, Level, StartTag)   mIsTraveling → WritePlayer → 입력 끄기 → 페이드 아웃 → 타이머
   OpenLevelURL(FString URL)                UGameplayStatics::OpenLevel(GetGameInstance(), URL)
   OnPostLoadMap(UWorld*)                   검정 고정 → ArriveHoldTime 타이머
   FadeIn()                                 mIsTraveling = false → 페이드 인
   GetSaveGame() const                      읽는 쪽이 nullptr이면 새 게임
   ▲ 읽기
   ├ UInventoryComponent::InitInventory     Save ? ReadFromSave(Save->Inventory) : GiveStartingItems()
   └ APlayerCharacter::InitAttributes       Save ? Save->Level : Info->Level → InitLevelStats → HP/MP/Exp/Gold 덮기
   ▼ 쓰기
   APlayerCharacter::WriteToSave(UElmSaveGame&) const   가방(UInventoryComponent::WriteToSave, public으로 옮김) + Attribute Base 값

UElmSaveGame (Save/ElmSaveGame.h)      SaveVersion, Inventory, Level, HP, MP, Exp, Gold
AElmGameModeBase                       ChoosePlayerStart_Implementation (태그 없는 PlayerStart가 기본 자리)
```

## 진행 상황

| 단계 | 내용 | 상태 |
| --- | --- | --- |
| 0 | 기본 GameMode 지정 (`GlobalDefaultGameMode`) | ☑ |
| 1 | `ALevelPortal` + Field ↔ OrcCave 포탈 배치 | ☑ |
| 2 | `USaveSubsystem` + 가방 유지 (+ 사용자가 추가한 `ChoosePlayerStart`) | ☑ |
| 3 | 능력치(Level / HP / MP / Exp / Gold) 유지 | ☑ |
| 4 | 페이드 아웃 / 인 + 연타 방지 | ◐ 코드는 안내와 똑같이 들어갔다. 빌드, PIE 테스트, 리뷰가 남았다 |
| 5 | (선택) 사망 → 병원에서 부활 | ☐ 설계 전 |

### 0 ~ 3단계 (완료)

코드는 `git diff`에 있다(아직 커밋 전). 결과는 결정 사항과 목표 구조에 정리했다. 확인한 내용:

| 확인 | 결과 |
| --- | --- |
| 모든 `ElmKingdom` 맵에서 PIE | 플레이어 캐릭터가 나온다 |
| Field 포탈 E ↔ 동굴 포탈 E 왕복 | Output Log `TravelToLevel : .../OrcCave#Field`, `.../Field#OrcCave`. 태그가 맞는 PlayerStart에 선다 |
| 가방 정렬 후 이동 | 칸 위치까지 유지 |
| HP / MP / Exp / Gold를 바꾼 뒤 이동 | 값 유지. PIE를 다시 시작하면 초기값(새 게임) |

### 4단계: 페이드 아웃 / 인 + 연타 방지

- 작업: [SaveSubsystem.h](../Source/ElmChronicles/Subsystem/SaveSubsystem.h), [SaveSubsystem.cpp](../Source/ElmChronicles/Subsystem/SaveSubsystem.cpp) 전체 교체. 포탈과 캐릭터 코드는 바꾸지 않는다. 두 파일 모두 이미 들어가 있다.
- 빌드: 헤더에 함수와 멤버가 늘었으므로 **에디터를 끄고** `Build.bat` (명령은 CLAUDE.md).
- 테스트:

| 확인 | 기대 결과 |
| --- | --- |
| Field에서 PIE 시작 | 페이드 없이 바로 보인다(`mIsTraveling`이 false라 `OnPostLoadMap`이 아무것도 안 한다) |
| 동굴 포탈에서 E | 0.5초 동안 어두워지고 그동안 WASD / 마우스 회전이 안 된다 → 로딩 → 동굴에서 0.5초 검은 화면 → 0.5초 동안 부드럽게 밝아진다 |
| 어두워지는 중 E 연타, I | 인벤토리가 안 열리고 `TravelToLevel` 로그가 1번만 나온다 |
| 동굴 도착 후 다시 포탈 | 정상 이동. 안 되면 `FadeIn`이 안 불려 `mIsTraveling`이 true로 남은 것 |
| 가방, HP, Gold | 3단계처럼 유지 |
| 소리(있다면) | 어두워지며 작아지고 밝아지며 돌아온다 |
| (선택 실험) `PC->DisableInput(PC)`를 잠시 주석 처리하고 어두워지는 중 E 연타 | 로그 1번 → `mIsTraveling`이 막고 있다. 확인 후 되돌린다 |
| (선택 실험) `ArriveHoldTime`을 `0.01f`로 | 도착 직후 화면이 번쩍 밝아진다. 확인 후 되돌린다 |

### 5단계 (선택): 사망 → 병원에서 부활

아직 설계 전이다. 지금 확인된 사실과 정할 것만 적는다.

- 이미 있는 것
  - `APlayerCharacter::OnKilledBy`: 입력을 끄고 이동을 멈춘다(로그 `Player Death`). 부활 처리는 없다.
  - `Content/Maps/ElmKingdom/Hospital.umap`: PlayerStart 2개, 포탈 없음.
  - `ANPCCharacter`의 부활 코드(HP를 먼저 채우고 `State.Dead`를 지움)가 참고가 된다.
  - `mIsTraveling` 덕분에 포탈로 이동하는 중에 사망 이동이 겹쳐도 한 번만 간다.
- 정할 것
  - 사망 후 이동까지 기다리는 시간과 연출.
  - 병원 맵 경로와 도착 태그를 어디에 둘지(코드 상수, GameMode 프로퍼티 등).
  - HP 0이 메모에 적히는 문제. 지금 `WriteToSave`는 HP 0을 그대로 적으므로, 병원에 도착하면 HP 0인 산 캐릭터가 된다. 부활 경로에서 HP / MP를 어떻게 채울지 정해야 한다.
  - 사망 페널티(골드, 경험치 감소 등). Plan.md에는 "마을 병원에서 다시 시작"만 있고 페널티 규칙은 없다. 사용자에게 묻는다.

## 에셋 작업

| 에셋 | 할 일 | 담당 | 상태 |
| --- | --- | --- | --- |
| `Field.umap` | `ALevelPortal`(→ OrcCave, 태그 `Field`) 배치. 동굴에서 돌아올 PlayerStart에 태그 `OrcCave` | 사용자 | ☑ |
| `OrcCave.umap` | `ALevelPortal`(→ Field, 태그 `OrcCave`) 배치. PlayerStart는 1개로 줄이고 태그 `Field` | 사용자 | ☑ |
| 나머지 맵(Hospital, KyleHouse, Palace, Shop, Tavern, VoidRaindungeon) | 포탈 배치와 PlayerStart 태그. 태그 규칙은 결정 사항 참고 | 사용자 | ☐ 범위 밖. 필요할 때 한다 |

## 진행 방식

- Claude는 코드를 직접 고치지 않는다(사용자가 "직접 해 줘"라고 할 때만). 단계마다 그대로 입력할 수 있는 전체 코드, 정확한 위치(파일, 함수 전체 교체 등), 테스트 표, 비유를 섞은 초보자용 설명을 준다.
- 코드 주석은 한글 "~한다" 체로, "왜 이렇게 하는지"를 적는다.
- 루프: 단계 안내 → 사용자가 입력, 빌드, PIE 테스트 → "Step N 완료" → Claude가 `git diff`와 파일로 리뷰 → 통과하면 이 문서의 상태를 ☑로 바꾸고(묻지 않는다) 다음 단계.
- 헤더나 `UPROPERTY` / `UCLASS`가 바뀌는 단계에서는 "에디터를 끄고 빌드"를 안내한다.
- 맵 배치 같은 에셋 작업은 사용자가 한다.

## 이후 작업 (이 계획의 범위 밖)

- 디스크 저장 / 불러오기(`SaveGameToSlot` / `LoadGameFromSlot`)와 새 게임 / 이어하기 메뉴. `USaveSubsystem::mSaveGame`을 그대로 넘기면 된다.
- 메모에 추가할 것: 장착 장비, 위치, 퀘스트 진행도(`ElmSaveGame.h`의 "나중에 추가" 주석).
- HUD까지 가리는 로딩 위젯.
- 나머지 맵의 포탈 배치.
