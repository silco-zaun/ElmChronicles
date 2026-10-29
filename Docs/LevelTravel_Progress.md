# 레벨 이동

> **상태**: 0 ~ 5단계 모두 완료(2026-10-29). 오래 남을 구조는 CLAUDE.md "레벨 이동 / 메모(SaveSubsystem)"로 옮겼다. 이 문서는 결정 이유와 함정 기록용으로 남아 있다. 이어지는 과제는 [이후 작업](#이후-작업-이-계획의-범위-밖).
>
> 마지막 갱신: 2026-10-29

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
| 태그 규칙 | **연결 이름**: 한 문(연결)의 양쪽 끝이 같은 태그를 쓴다. Field ↔ 동굴은 `OrcCave#OrcCave` / `Field#OrcCave`. 예외: 사망 후 병원은 고정 태그 `Respawn` | 2026-10-29 변경. 처음 규칙은 "도착 PlayerStart 태그 = 출발 맵 이름"이었으나, 사용자가 나머지 맵을 배치하면서 연결 이름 방식(`Hospital`, `HeroHouse`, `Castle`)을 썼고, 이 방식은 같은 두 맵 사이의 문 여러 개도 구분한다. Field ↔ OrcCave(`Field` / `OrcCave`)도 `OrcCave`로 통일했다. 사망과 메뉴 "귀환"(KyleHouse `Room`)은 어느 맵에서나 일어나므로 연결 이름이 없다 |
| 태그 없이 들어왔을 때 | `AElmGameModeBase::ChoosePlayerStart_Implementation`: `APlayerStartPIE`(뷰포트 "Play From Here")가 있으면 그것, 없으면 태그 없는 첫 PlayerStart, 그것도 없으면 엔진 기본 | 엔진 기본은 무작위라 PIE를 시작할 때마다 서는 자리가 바뀌었다. 사용자가 직접 추가했다 |
| 메모 보관 | `USaveSubsystem`(GameInstanceSubsystem)이 `UPROPERTY() TObjectPtr<UElmSaveGame> mSaveGame`을 든다. nullptr = 새 게임 | 나중에 디스크 저장도 같은 객체를 `SaveGameToSlot`에 넘기면 된다 |
| 쓰기 / 읽기 위치 | 쓰기는 한 곳(`TravelToLevel` → `WritePlayer` → `APlayerCharacter::WriteToSave`). 읽기는 부품마다 자기 초기화 시점(`UInventoryComponent::InitInventory`, `APlayerCharacter::InitAttributes`) | 초기화 함수가 이미 `CallOrRegisterDataLoaded`를 거치므로 테이블이 준비된 뒤에 읽는다. 새 레벨에서는 데이터가 이미 로드돼 있어 즉시 불린다 |
| 쓰는 시점 | E를 누른 순간(페이드 아웃 시작 전) | "문을 나선 순간"의 상태를 기준으로 한다. 어두워지는 0.5초 동안 생긴 변화는 적지 않는다 |
| 능력치 범위 | Level, HP, MP, Exp, Gold만 적는다 | MaxHP, 공격력 등은 Level로 `DT_LevelStat`에서 다시 계산된다 |
| 능력치 읽기 / 쓰기 | `GetNumericAttributeBase` / `SetNumericAttributeBase` (Base 값) | 지속형 GE의 버프(장비 등)가 저장값에 섞이지 않는다 |
| 화면 전환 담당 | `USaveSubsystem`이 페이드 아웃(`TravelToLevel`)과 페이드 인(`OnPostLoadMap` → `FadeIn`)을 모두 맡는다 | 버린 안: PlayerController / 카메라. 레벨과 함께 사라져서 "아까 어둡게 했다"는 사실을 도착 레벨로 넘길 수 없다 |
| 도착 알림 | `FCoreUObjectDelegates::PostLoadMapWithWorld`. `Initialize`에서 등록, `Deinitialize`에서 `RemoveAll` | 엔진 `UEngine::LoadMap`이 플레이어 스폰 → `World->BeginPlay()` → 이 알림 순서라서, 알림 시점에 새 컨트롤러와 카메라가 이미 있다(2026-10-29 엔진 소스 `UnrealEngine.cpp`로 확인. `UEditorEngine`은 `LoadMap`을 override하지 않아 PIE도 같다). 전역 알림이라 `World->GetGameInstance() != GetGameInstance()`인 월드는 거른다 |
| 연타 / 중복 이동 방지 | 두 겹. 입력 끄기(`Player->DisableInput(PC)` + `PC->DisableInput(PC)`)와 `mIsTraveling` | 입력을 끄면 E가 안 들어오고, `mIsTraveling`은 키가 아닌 경로(5단계의 사망 이동 타이머 등)까지 막는다. 새 레벨에서는 Pawn과 컨트롤러가 새로 만들어지므로 입력을 다시 켤 필요가 없다 |
| 페이드 시간 | 페이드 아웃 0.5초 → `OpenLevel` → 도착 후 검은 화면 유지 0.5초(`ArriveHoldTime`) → 페이드 인 0.5초 | 유지 시간이 필요한 이유는 [주의](#주의)의 "도착 직후 화면이 번쩍 켜짐" |
| 사망 후 다시 시작 담당 | `APlayerCharacter::OnKilledBy` → `AElmGameModeBase::PlayerKilled` → `RespawnDelay` 타이머 → `RespawnPlayer` → `USaveSubsystem::TravelToLevel` | 어디서 언제 다시 시작할지는 게임 규칙이라 GameMode의 일이다. 캐릭터는 죽었다고 알리기만 한다. 화면 전환, 메모 적기, 중복 이동 방지는 포탈과 같은 `TravelToLevel`을 그대로 쓴다 |
| 사망 후 대기 | `RespawnDelay` 2초(쓰러지는 애니메이션을 보여 줌) → 포탈과 같은 페이드 | 버린 안: 사망 애니메이션의 `AnimNotify_DeathEnd`에 맞추기. `ABP_Player`에 DeathEnd 노티파이가 없고(2026-10-29 확인), 노티파이가 안 울리면 죽은 채로 갇힌다 |
| 병원 맵 / 도착 자리 | `AElmGameModeBase` 생성자에서 `mRespawnLevel` = `/Game/Maps/ElmKingdom/Hospital.Hospital`, `mRespawnStartTag` = `Respawn` | 태그를 단 PlayerStart가 없으면 엔진 `FindPlayerStart`가 `ChoosePlayerStart`(태그 없는 첫 PlayerStart)로 넘어가므로 태그를 달기 전에도 동작한다 |
| 부활(HP / MP 채우기) | 메모의 HP가 0이면 `InitAttributes`가 HP / MP를 메모 값으로 덮지 않는다. `InitLevelStats`가 가득 채운 값이 남는 것이 부활이다 | 산 캐릭터는 HP가 0일 수 없어서(사망은 HP 0에서 발동, HP 비용은 `CurrentHP > mHPCost`일 때만 냄) "메모 HP 0 = 죽어서 왔다"가 정확하다. 메모에 부활 플래그를 따로 두지 않는다. 새 레벨은 PlayerState와 캐릭터를 새로 만들므로 `State.Dead` 제거나 이동 재개 같은 부활 코드도 필요 없다(`ANPCCharacter::Revive`와 다른 점) |
| 사망 페널티 | 없다. Exp, Gold, 가방 모두 유지 | Plan.md에 규칙이 없다. 2026-10-29 Claude의 추천안(페널티 없음, 약 2초 대기)으로 사용자가 진행을 지시했다 |
| 죽은 뒤 상호작용 | `AElmPlayerController::InteractKey`가 `PlayerCharacter->IsDead()`면 무시 | 아래 주의 "죽은 직후 포탈에서 E" |

## 주의

| 증상 | 원인 | 해결 |
| --- | --- | --- |
| Grep(rg)으로 `.umap`에서 `GameMode`를 찾으면 아무것도 안 나와서 "참조 없음"으로 오판 | rg는 바이너리 파일을 건너뛴다 | `grep -aoE "<문자열>" <맵.umap>` 처럼 `-a`로 바이너리를 텍스트로 읽는다. PlayerStart 태그가 있는지는 `grep -ac "PlayerStartTag" <맵.umap>`(0이면 태그 단 PlayerStart가 없다) |
| PIE에서 Output Log에 `TravelToLevel : /Game/Maps/ElmKingdom/UEDPIE_0_OrcCave#Field` | PIE가 이미 로드한 맵을 가리키는 Soft 경로에 `FSoftObjectPath::FixupForPIE`가 `UEDPIE_0_` 접두사를 붙인다 | 정상이다. `LoadMap`이 접두사를 떼고 정상 맵을 연다. 패키징 빌드에서는 붙지 않는다 |
| 이동 후 HP / MP가 0이 됨(가능성) | `PreAttributeBaseChange`가 HP / MP를 MaxHP / MaxMP로 Clamp한다. Max가 0일 때 HP를 넣으면 0으로 잘린다 | `InitAttributes`에서 `InitLevelStats`(Max를 넣고 HP / MP를 가득 채움) 다음에 메모 값을 덮는다. 순서를 바꾸지 않는다 |
| 도착 직후 화면이 한 프레임 만에 확 밝아짐 | 로딩이 끝난 다음 프레임의 DeltaTime에 로딩 시간이 한꺼번에 들어온다. `WorldSettings`의 `MaxUndilatedFrameTime`(BaseGame.ini 0.4초)까지 잘리지만, 이 프로젝트의 `LoadMap`은 0.25 ~ 0.65초가 걸려서(`Saved/Logs`의 `Took ... seconds to LoadMap`) 0.5초 페이드가 한 프레임에 50 ~ 80% 진행된다 | 도착하면 `SetManualCameraFade(1)`로 검정을 고정하고 `ArriveHoldTime`(0.5초) 뒤에 페이드 인을 시작한다 |
| 페이드 아웃이 끝나고 `OpenLevel` 전에 화면이 번쩍 | `StartCameraFade`의 `bHoldWhenFinished`가 false면 끝나는 순간 `StopCameraFade`로 페이드가 꺼진다 | 페이드 아웃은 6번째 인자 `true`(검정 유지) |
| `ArriveHoldTime`을 0으로 하면 검은 화면에서 멈춤 | 시간이 0 이하인 타이머는 걸리지 않아 `FadeIn`이 불리지 않는다 | 0보다 큰 값을 쓴다 |
| `FTimerDelegate::CreateUObject(this, &F, URL)` 컴파일 에러 | Payload를 받는 매개변수가 참조(`const FString&`)면 안 된다 | `OpenLevelURL(FString URL)`처럼 값으로 받는다(`AssetSubsystem::OnGameDataLoaded`와 같다). 5단계의 `RespawnPlayer(TWeakObjectPtr<APlayerCharacter>)`도 값으로 받는다 |
| 캐릭터 입력만 끄면 E / I가 계속 동작 | `APlayerController::BuildInputStack`이 Pawn 입력과 컨트롤러 입력의 켜짐 상태를 따로 본다 | 둘 다 `DisableInput` |
| 검은 화면 위에 HP 바, `[E] 이동하기` 문구가 보임 | 카메라 페이드는 3D 화면에 색을 덧씌우는 방식(`ViewInitOptions.OverlayColor`)이라 UMG는 덮지 않는다 | 지금은 그대로 둔다. 가리려면 화면 전체를 덮는 로딩 위젯으로 바꾼다. `TravelToLevel`과 `FadeIn` 두 곳만 바꾸면 된다 |
| `PostLoadMapWithWorld`가 `nullptr` 월드로 불림 | `UEngine::LoadMap`이 실패하면 `FPostLoadMapCaller` 소멸자가 `nullptr`로 알린다 | `OnPostLoadMap`의 `!IsValid(World)`가 거른다. 이미 처리돼 있다 |
| 죽은 직후 포탈 앞에서 E → 죽은 채로 이동하고 메모에 HP 0이 적힘 | 플레이어가 죽어도 `OnKilledBy`는 Pawn 입력만 끈다(컨트롤러의 E는 살아 있음). `UpdateInteractTarget`은 죽으면 대상을 비우지만 0.1초마다라서, 그 사이에는 `mInteractTarget`에 포탈이 남아 있다 | `InteractKey` 조건에 `PlayerCharacter->IsDead()`를 넣는다(5단계에 포함) |
| 에디터를 켠 채 IDE / UBT로 빌드하면 `UnrealEditor-ElmChronicles-000N.dll`이 생기고 Hot Reload를 시도 | Live Coding이 시작되지 않은 에디터에서는 UBT가 빌드를 막지 않는다. 5단계 빌드도 이렇게 들어갔다(2026-10-29 로그 `Starting Hot-Reload from IDE`, `-0001.dll`). 멤버 추가뿐이라 이번에는 문제없었다 | 헤더를 바꾼 단계는 에디터를 끄고 `Build.bat`으로 빌드한다(CLAUDE.md "빌드 / 실행") |

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
   └ APlayerCharacter::InitAttributes       Save ? Save->Level : Info->Level → InitLevelStats
                                            → (Save->HP > 0일 때만) HP/MP 덮기 → Exp/Gold 덮기
   ▼ 쓰기
   APlayerCharacter::WriteToSave(UElmSaveGame&) const   가방(UInventoryComponent::WriteToSave, public으로 옮김) + Attribute Base 값

AElmGameModeBase (GameModes/)
   ChoosePlayerStart_Implementation          태그 없는 PlayerStart가 기본 자리
   mRespawnLevel / mRespawnStartTag          Hospital / "Respawn"
   PlayerKilled(APlayerCharacter*)           ← APlayerCharacter::OnKilledBy. RespawnDelay(2초) 타이머
   RespawnPlayer(TWeakObjectPtr<APlayerCharacter>)   → USaveSubsystem::TravelToLevel(Player, mRespawnLevel, mRespawnStartTag)

UElmSaveGame (Save/ElmSaveGame.h)      SaveVersion, Inventory, Level, HP, MP, Exp, Gold
```

## 진행 상황

| 단계 | 내용 | 상태 |
| --- | --- | --- |
| 0 | 기본 GameMode 지정 (`GlobalDefaultGameMode`) | ☑ |
| 1 | `ALevelPortal` + Field ↔ OrcCave 포탈 배치 | ☑ |
| 2 | `USaveSubsystem` + 가방 유지 (+ 사용자가 추가한 `ChoosePlayerStart`) | ☑ |
| 3 | 능력치(Level / HP / MP / Exp / Gold) 유지 | ☑ |
| 4 | 페이드 아웃 / 인 + 연타 방지 | ☑ |
| 5 | (선택) 사망 → 병원에서 부활 | ☑ (2026-10-29, 미커밋) |

### 0 ~ 4단계 (완료)

코드는 커밋 `7d86c3e 저장소 이동`에 들어 있다(이 커밋에서 저장소가 `C:\Tadi\Work\ElmChronicles`로 옮겨졌다). 결과는 결정 사항과 목표 구조에 정리했다. 확인한 내용:

| 확인 | 결과 |
| --- | --- |
| 모든 `ElmKingdom` 맵에서 PIE | 플레이어 캐릭터가 나온다 |
| Field 포탈 E ↔ 동굴 포탈 E 왕복 | Output Log `TravelToLevel : .../OrcCave#Field`, `.../Field#OrcCave`. 태그가 맞는 PlayerStart에 선다 |
| 가방 정렬 후 이동 | 칸 위치까지 유지 |
| HP / MP / Exp / Gold를 바꾼 뒤 이동 | 값 유지. PIE를 다시 시작하면 초기값(새 게임) |
| 4단계 페이드 / 연타 방지 | 2026-10-29 PIE 로그에서 왕복 이동, E 한 번에 `TravelToLevel` 1번, `LoadMap` 0.29 ~ 0.65초. 코드 리뷰 통과 후 사용자가 5단계 진행을 지시했다 |

### 5단계 (완료): 사망 → 병원에서 부활

코드는 아직 커밋 전(`git diff`)이다. 바꾼 곳:

| 파일 | 바꾼 곳 |
| --- | --- |
| [ElmGameModeBase.h](../Source/ElmChronicles/GameModes/ElmGameModeBase.h) / [.cpp](../Source/ElmChronicles/GameModes/ElmGameModeBase.cpp) | `mRespawnLevel`, `mRespawnStartTag`, `RespawnDelay`(2초), `PlayerKilled`(타이머), `RespawnPlayer`(`TravelToLevel` 호출) |
| [PlayerCharacter.cpp](../Source/ElmChronicles/Characters/PlayerCharacter.cpp) | `OnKilledBy` 끝에 `GameMode->PlayerKilled(this)`. `InitAttributes`의 HP / MP 덮기를 `Save->HP > 0.f`일 때만 |
| [ElmPlayerController.cpp](../Source/ElmChronicles/Player/ElmPlayerController.cpp) | `InteractKey` 조건에 `PlayerCharacter->IsDead()` |

확인한 내용(2026-10-29 PIE, 사용자 확인 + `Saved/Logs/ElmChronicles.log`):

| 확인 | 결과 |
| --- | --- |
| Field에서 몬스터에게 죽음(2회) | `Player Death - Killer : MonsterCharacter_0` 정확히 2.0초 뒤 `TravelToLevel : /Game/Maps/ElmKingdom/Hospital#Respawn` 한 번, `LoadMap` 0.26초 |
| 병원 도착 후 HP / MP, Exp / Gold / 가방, 죽은 직후 E 연타 | 테스트 표대로 동작(사용자 확인) |
| 병원 → Field 포탈 | `TravelToLevel : .../UEDPIE_0_Field#Hospital` 정상 이동 |

리뷰 때 실패 원인 후보로 적어 둔 것: 죽어도 이동하지 않으면 `GetAuthGameMode<AElmGameModeBase>()`가 null인지 본다(그 맵 World Settings의 GameMode override). 도착 후 HP가 0이면 `InitAttributes`의 `Save->HP > 0.f` 조건을 본다.

## 에셋 작업

| 에셋 | 할 일 | 담당 | 상태 |
| --- | --- | --- | --- |
| `Field.umap` | `ALevelPortal`(→ OrcCave, 태그 `OrcCave`) 배치. 동굴에서 돌아올 PlayerStart에 태그 `OrcCave` | 사용자 | ☑ (2026-10-29 포탈 태그 `Field` → `OrcCave`, Claude가 unreal-mcp로) |
| `OrcCave.umap` | `ALevelPortal`(→ Field, 태그 `OrcCave`) 배치. PlayerStart는 1개로 줄이고 태그 `OrcCave` | 사용자 | ☑ (2026-10-29 PlayerStart 태그 `Field` → `OrcCave`, Claude가 unreal-mcp로) |
| `Hospital.umap` | 부활할 PlayerStart 하나에 Player Start Tag `Respawn`. Field로 가는 포탈 | 사용자 | ☑ (`PlayerStartTag`와 `Respawn` 이름이 맵에 있음, 포탈 `LevelPortal_1`) |
| KyleHouse, Palace, Shop, Tavern | Field와 오가는 포탈과 PlayerStart 태그 | 사용자 + Claude | ☑ 2026-10-29. 사용자가 Field 쪽 포탈 / PlayerStart, KyleHouse 포탈, 각 맵 PlayerStart를 배치했다. Claude가 unreal-mcp로 고친 것: KyleHouse 포탈 `mTargetLevel` KyleHouse → Field, Shop / Tavern PlayerStart 태그 `Shop` / `Tavern`, Shop / Tavern / Palace에 Field로 가는 포탈 추가(`Exit_ToTown` 앞 (x, 100, 105), Box Extent 100). 12개 연결 모두 태그가 맞는 PlayerStart가 있음을 저장본으로 교차 확인. PIE 왕복은 아직 안 했다 |
| KyleHouse `PlayerStart_House`(태그 `Room`) | 나중에 만들 메뉴 UI의 "귀환"이 도착할 자리(사용자 결정 2026-10-29). 포탈이 쓰지 않는 고정 태그라 `Respawn`과 같은 예외다 | 사용자 | ☑ 배치됨. 메뉴는 아직 없다 |
| VoidRaindungeon | 연결 없음(`Exit_ToField` / `Arrive_FromField` 표식만 있다) | 사용자 | ☐ 범위 밖 |

## 진행 방식

- Claude는 코드를 직접 고치지 않는다(사용자가 "직접 해 줘"라고 할 때만). 단계마다 그대로 입력할 수 있는 전체 코드, 정확한 위치(파일, 함수 전체 교체 등), 테스트 표, 비유를 섞은 초보자용 설명을 준다.
- 코드 주석은 한글 "~한다" 체로, "왜 이렇게 하는지"를 적는다.
- 루프: 단계 안내 → 사용자가 입력, 빌드, PIE 테스트 → "Step N 완료" → Claude가 `git diff`와 파일로 리뷰 → 통과하면 이 문서의 상태를 ☑로 바꾸고 다음 단계.
- 이 문서와 CLAUDE.md의 갱신은 사용자에게 묻지 않고 Claude가 한다(2026-10-29 사용자 지시).
- 헤더나 `UPROPERTY` / `UCLASS`가 바뀌는 단계에서는 "에디터를 끄고 빌드"를 안내한다.
- 맵 배치 같은 에셋 작업은 사용자가 한다.

## 이후 작업 (이 계획의 범위 밖)

- 디스크 저장 / 불러오기(`SaveGameToSlot` / `LoadGameFromSlot`)와 새 게임 / 이어하기 메뉴. `USaveSubsystem::mSaveGame`을 그대로 넘기면 된다.
- 메모에 추가할 것: 장착 장비, 위치, 퀘스트 진행도(`ElmSaveGame.h`의 "나중에 추가" 주석).
- HUD까지 가리는 로딩 위젯.
- VoidRaindungeon 연결.
- 메뉴 UI "귀환": `USaveSubsystem::TravelToLevel(Player, KyleHouse, "Room")`. → [MenuUI_Progress.md](MenuUI_Progress.md)에서 진행. 사망 후 병원 이동처럼 맵 / 태그를 GameMode에 두면(`mRespawnLevel` / `mRespawnStartTag`와 같은 방식) 메뉴는 "귀환해 줘"라고 부르기만 하면 된다. 화면 전환, 메모 적기, 중복 이동 방지는 `TravelToLevel`이 이미 한다.
- 사망 페널티(Plan.md에 규칙이 생기면). `TravelToLevel`이 메모를 적기 전에 깎아야 하므로 `RespawnPlayer`에서 `TravelToLevel`을 부르기 전에 Attribute를 바꾼다.
- 사망 애니메이션 길이에 맞춘 이동. `ABP_Player` 사망 애니메이션에 DeathEnd 노티파이를 넣고 `UPlayerAnimInstance::AnimNotify_DeathEnd`(지금 비어 있음) → `APlayerCharacter::Death()`(지금 비어 있음)에서 GameMode에 알린다. 노티파이가 안 울릴 때를 대비한 타이머는 남긴다.
