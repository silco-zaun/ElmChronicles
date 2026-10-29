# NPC 대화

> **다음 세션 시작 방법**: 이 문서를 읽고 → `git status` / `git diff`로 어디까지 했는지 확인하고 → [진행 상황](#진행-상황) 표에서 첫 번째 ☐ 단계부터 이어서 한다.
>
> 마지막 갱신: 2026-10-27

## 목표

플레이어가 NPC 앞에서 E를 누르면 화면 아래에 대화창(말하는 사람 + 대사)이 뜨고, E / Space / 마우스 왼쪽으로 다음 대사로 넘긴다. 마지막 대사에서 넘기면 대화가 끝나고 원래 조작으로 돌아온다. 대화 중에는 이동, 공격, 카메라, 인벤토리가 막히고, NPC는 하던 행동을 멈추고 플레이어를 바라본다.

## 배경

상호작용까지는 이미 되어 있다. 이번 작업은 그 뒤를 잇는다.

| 이미 있는 것 | 위치 |
| --- | --- |
| 상호작용 인터페이스 `IInteractable` (`GetInteractText` / `CanInteract` / `Interact`) | [Interaction/Interactable.h](../Source/ElmChronicles/Interaction/Interactable.h) |
| 0.1초마다 주변 대상 검색, E 키 → `Interact` 호출 | `AElmPlayerController::UpdateInteractTarget` / `InteractKey` ([ElmPlayerController.cpp](../Source/ElmChronicles/Player/ElmPlayerController.cpp)) |
| 안내 문구 `[E] 대화하기` | `UPlayerHUDWidget::SetInteractPrompt`, WBP의 `mInteractText` |
| NPC의 대화 시작 행 | `FNPCCharacterTableRow::DialogueRow` → `ANPCCharacter::mDialogueRow` (`InitAttributes`에서 읽음). `DT_NPC`의 `TavernKeeper` 행은 `Tavern_01`, `Villager`는 None |
| 대화 가능 조건 | `ANPCCharacter::CanInteract`: `mDialogueRow`가 있고, 살아 있고, BB `Target`이 비어 있음(전투 중이 아님) |

`ANPCCharacter::Interact`는 지금 로그 한 줄만 찍는다("5-2에서 여기서 대화를 시작한다").

이 작업의 처음 설계는 삭제된 NPC 진행 문서의 "Step 5 계획"에 있다: `git show aca5af2^:./Docs/NPC_Progress.md`. 이 문서는 그 계획을 지금 구조(`AAICharacter`, `UElmGameplayAbility` 통합 이후)에 맞춘 것이다.

## 결정 사항

| 항목 | 결정 | 이유 / 버린 안 |
| --- | --- | --- |
| 대화 데이터 | `DT_Dialogue`(`FDialogueTableRow`). 대사 한 줄이 한 행이고 `NextRow`로 다음 행을 잇는다. `NextRow`가 None이면 끝 | 행 이름이 `FName`이라 `FindRow`에 바로 넣을 수 있다 |
| 선택지 | 없음 (선형 대화) | Plan.md의 퀘스트는 모두 "말 걸고 의뢰 받기"라 선택지가 필요 없다. 필요해지면 행에 열을 추가한다 |
| 글자 타입 | `Speaker`, `Text`는 `FString` | 버린 안: `FText`. 다른 테이블(`Name`, `Description`)과 `dt-csv-sync` 스크립트가 `FString` 기준이다. 번역이 필요해지면 바꾼다 |
| `Speaker` | 대화창에 그대로 보일 이름을 적는다 ("선술집 주인", "카일") | NPC 이름(DT_NPC `Name`)과 자동 연결하지 않는다. 플레이어 대사도 같은 열로 처리하려고 |
| 대화 상태를 들고 있는 곳 | `AElmPlayerController` (`mDialogueNPC`, `mDialogueRow`) | 버린 안: 별도 컴포넌트 / 서브시스템. HUD, 입력 모드, 상호작용 대상이 이미 이 컨트롤러에 있어서 따로 빼면 서로를 찾아다녀야 한다 |
| 대화 중 입력 | `IMC_Default`를 빼고 `IMC_Dialogue`(E / Space / 마우스 왼쪽 → `IA_DialogueNext`)를 넣는다 | 버린 안: `SetIgnoreMoveInput` 등으로 하나씩 막기. 공격, 점프, 카메라, 인벤토리를 각각 막아야 하고, 입력이 늘 때마다 빠뜨리기 쉽다 |
| 커서 | 대화 중에는 커서를 띄우지 않는다. `IsAnyWindowOpen`에 대화창을 넣지 않는다 | 마우스 왼쪽 클릭으로 넘기므로 커서가 필요 없다. `IsAnyWindowOpen`이 true면 `RefreshInputMode`가 GameAndUI + 커서로 바꾼다 |
| 대화 시작 시 정리 | 인벤토리를 닫고, 안내 문구를 숨기고, `mInteractTarget`을 비운다 | |
| NPC 쪽 처리 | `ANPCCharacter::BeginTalk(Listener)`: BT `PauseLogic` → `StopMovement` → `SetFocus(Listener)`. `EndTalk()`: `ClearFocus(Gameplay)` → `ResumeLogic` | 버린 안: BT는 그대로 두고 `SetFocus`만 하기. 순찰(스포너의 `mPatrolPath`)이나 귀가(`HomeLocation`) 중이면 말하다가 걸어가 버린다. NPC 처리는 컨트롤러가 아니라 NPC가 한다 |
| 플레이어 쪽 처리 | 대화를 시작할 때 NPC 방향으로 몸을 돌린다(Yaw만, `SetActorRotation`) | 플레이어는 `bOrientRotationToMovement`라서 입력이 없으면 스스로 돌지 않는다 |
| 대화가 끊기는 조건 | ① 대화 중에는 0.1초 타이머(`UpdateInteractTarget`)가 새 대상을 찾지 않고, 플레이어가 죽었거나 NPC가 사라졌거나 `CanInteract`가 false면 `EndDialogue`. ② 플레이어가 맞으면(`APlayerCharacter::OnDamagedBy`) `EndDialogue` | `CanInteract`를 다시 쓰면 NPC가 쓰러지거나 전투에 들어가는 경우를 한 번에 처리한다. 멀리서 날아온 투사체처럼 NPC가 못 본 공격은 ②가 잡는다. 플레이어 사망은 `OnDamagedBy`가 아니라 `OnKilledBy`로 가므로 ①이 잡는다 |

## 주의

| 증상 | 원인 | 해결 |
| --- | --- | --- |
| 대화 중에 죽은 필수 NPC(TavernKeeper)가 부활한 뒤 가만히 서 있음 | `UBehaviorTreeComponent::StopTree`(사망 시 `OnKilledBy`가 부름)는 `bIsPaused`를 풀지 않는다. `ProcessExecutionRequest`는 `bIsPaused`면 실행을 무시하므로, `Revive`의 `RestartLogic` 뒤에도 BT가 멈춰 있다 (엔진 `BehaviorTreeComponent.cpp`의 `PauseLogic` / `ResumeLogic` / `ProcessExecutionRequest`에서 확인) | `EndTalk`는 NPC가 죽었어도 **반드시** `ResumeLogic`을 부른다. `IsDead()`로 건너뛰지 않는다 |
| (예방) 공격 태스크가 건 포커스가 지워짐 | `BTTask_MonsterAttack`도 `SetFocus(Target)`(Gameplay 우선순위)를 쓴다 | `EndTalk`에서 `ClearFocus` → `ResumeLogic` 순서를 지킨다. 전투로 대화가 끊길 때는 BT가 아직 멈춰 있어서 공격 태스크가 포커스를 걸기 전이다 |
| (예방) E로 대화를 시작하자마자 첫 대사가 넘어감 | E가 눌린 채로 IMC가 바뀐다 | `FModifyContextOptions`의 기본값 `bIgnoreAllPressedKeysUntilRelease = true`가 키를 뗄 때까지 무시해 준다. `AddMappingContext` / `RemoveMappingContext`에 옵션을 따로 넘기지 않는다 |
| DT 임포트에서 Row Type 목록이 이상하거나 임포트가 깨짐 | 에디터를 켠 채로 IDE에서 빌드하면 Hot Reload가 일어나 `HOTRELOAD_` 구조체가 생긴다 | 새 `USTRUCT` / `UCLASS` / `UPROPERTY`가 있는 단계는 에디터를 끄고 빌드한다 |
| `Content/Input/IA_Interaction`과 `IA_Interact`가 둘 다 있음 | `IMC_Default`가 참조하는 것은 `IA_Interact`다. `IA_Interaction`은 어디서도 참조하지 않는 잔재다 | `IMC_Dialogue`를 만들 때 헷갈리지 않는다. 지우는 것은 사용자 판단 |
| CSV를 Write / Edit로 고치면 글자가 깨짐 | CSV가 UTF-16 LE + BOM이다 | `dt-csv-sync` 스크립트로만 만들고 고친다 |

## 목표 구조

```
E 키 → AElmPlayerController::InteractKey → ANPCCharacter::Interact
        → AElmPlayerController::StartDialogue(NPC, mDialogueRow)
             ├ NPC->BeginTalk(플레이어)      BT 일시정지, 멈춤, 플레이어 바라보기
             ├ 플레이어를 NPC 쪽으로 회전
             ├ 인벤토리 닫기, 안내 문구 숨기기
             ├ SetDialogueInput(true)        IMC_Default → IMC_Dialogue
             └ ShowDialogueLine()            HUD 대화창에 현재 행 표시
IA_DialogueNext → AdvanceDialogue: NextRow가 있으면 ShowDialogueLine, None이면 EndDialogue
EndDialogue: 대화창 숨기기, IMC_Dialogue → IMC_Default, NPC->EndTalk(), 상태 비우기
```

```cpp
// Data/Tables/DialogueTableRow.h (헤더만, .cpp 없음)
USTRUCT(BlueprintType)
struct FDialogueTableRow : public FTableRowBase
{
	FString Speaker;
	FString Text;
	FName NextRow;   // None이면 끝
};

// UI/Game/DialogueWidget.h
UCLASS() class UDialogueWidget : public UElmBaseWidget
{
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> mSpeakerText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> mDialogueText;
	void SetLine(const FString& Speaker, const FString& Text);
};

// UPlayerHUDWidget에 추가
UPROPERTY(meta = (BindWidget)) TObjectPtr<class UDialogueWidget> mDialogueWidget;
void SetDialogueLine(const FString& Speaker, const FString& Text);
void SetDialogueVisible(bool Visible);

// UDefaultInput에 추가: IMC_Dialogue, 그리고 mInputActionMap에 "DialogueNext"
UPROPERTY() TObjectPtr<UInputMappingContext> mDialogueMappingContext;

// AElmPlayerController에 추가
TWeakObjectPtr<class ANPCCharacter> mDialogueNPC;
FName mDialogueRow;
void StartDialogue(ANPCCharacter* NPC, FName StartRow);
void AdvanceDialogue();
void EndDialogue();
bool IsInDialogue() const;
private: void ShowDialogueLine(); void SetDialogueInput(bool Dialogue);

// ANPCCharacter에 추가
void BeginTalk(APawn* Listener);
void EndTalk();
```

## 진행 상황

| 단계 | 내용 | 상태 |
| --- | --- | --- |
| Step 1 | 대화 데이터: `FDialogueTableRow`, `DT_Dialogue.csv`, DT 임포트, 로그로 확인 | ☑ 코드 리뷰 통과, 빌드 확인, CSV 생성 (2026-10-27). DT 임포트 / DA 등록 / 로그 테스트는 사용자 몫 |
| Step 2 | 대화창 UI: `UDialogueWidget`, `WBP_Dialogue`, HUD 연결 | ☐ (2026-10-27 전체 코드 안내함) |
| Step 3 | 대화 진행: `IMC_Dialogue`, `StartDialogue` / `AdvanceDialogue` / `EndDialogue` | ☐ |
| Step 4 | 대화 중 처리: NPC `BeginTalk` / `EndTalk`, 마주 보기, 끊기는 조건 | ☐ |

사전 조건: 레벨에 `NPCCharacter`를 놓고 Details의 **M Row Name**을 `TavernKeeper`로 정해 둔다.

### Step 1: 대화 데이터

- 작업
  - 새 파일 `Source/ElmChronicles/Data/Tables/DialogueTableRow.h` (위 목표 구조. 헤더만)
  - [NPCCharacter.cpp](../Source/ElmChronicles/Characters/NPCCharacter.cpp): `#include "../Data/Tables/DialogueTableRow.h"` 추가. **임시 테스트**로 `Interact`가 `mDialogueRow`부터 `NextRow`를 따라가며 `UE_LOG(LogTestDebug, Warning, ...)`로 대사를 찍는다. 고리 방지를 위해 최대 20줄. `UAssetSubsystem`은 `AICharacter.h`가 이미 include한다. Step 3에서 이 코드를 지운다
  - 빌드 리뷰가 끝나면 Claude가 `DataSource/DT_Dialogue.csv`를 만든다: `sync DT_Dialogue --struct FDialogueTableRow` → `add-row` 5번
- 빌드: 새 `USTRUCT`이므로 에디터를 끄고 IDE에서 빌드
- CSV 내용 (Plan.md 퀘스트 3: 선술집, 고블린 토벌 보상 1000골드 + 포션)

| 행 | Speaker | Text | NextRow |
| --- | --- | --- | --- |
| `Tavern_01` | 선술집 주인 | 어서 오게, 카일! 왕실 기사를 그만뒀다는 소문은 들었네. | `Tavern_02` |
| `Tavern_02` | 카일 | 이제 먹고살 일을 찾아야지. 들어온 의뢰가 있나? | `Tavern_03` |
| `Tavern_03` | 선술집 주인 | 마침 잘 왔네. 왕국 근처에 고블린 무리가 나타나 상인들을 괴롭힌다더군. | `Tavern_04` |
| `Tavern_04` | 선술집 주인 | 녀석들을 처치해 주면 1000골드에 포션도 넉넉히 얹어 주지. | `Tavern_05` |
| `Tavern_05` | 카일 | 맡겨 두게. 금방 다녀오지. | None |

- 에디터 작업(사용자): CSV를 `Content/Data`에 드래그 → Import As `DataTable`, Row Type **`DialogueTableRow`** → `DT_Dialogue`. `DA_GameData` → `mTables`에 `DT_Dialogue` 추가 → Save

| 확인 | 기대 결과 |
| --- | --- |
| `DT_Dialogue` 열기 | 5행, 열은 Speaker / Text / NextRow |
| TavernKeeper 앞에서 E | Output Log에 `[Tavern_01] 선술집 주인 : …`부터 `[Tavern_05]`까지 순서대로 5줄 |
| Villager 앞 | `[E] 대화하기`가 뜨지 않음 |
| 로그 `DialogueTableRow 테이블이 등록되지 않았습니다` | `DA_GameData`에 `DT_Dialogue`가 없거나 저장 안 함 |
| 로그 `requested row 'Tavern_0X' not in DataTable` | 행 이름 오타. CSV와 `DT_NPC`의 `DialogueRow` 비교 |

### Step 2: 대화창 UI

- 작업
  - 새 파일 `Source/ElmChronicles/UI/Game/DialogueWidget.h/.cpp`: `UElmBaseWidget` 자식. `SetLine`이 두 TextBlock에 글자를 넣는다
  - [PlayerHUDWidget.h/.cpp](../Source/ElmChronicles/UI/Game/PlayerHUDWidget.h): `mDialogueWidget`(BindWidget), `SetDialogueLine`(글자 넣고 보이기), `SetDialogueVisible`. `NativeOnInitialized`에서 숨긴다. 보일 때는 `HitTestInvisible`로 해서 클릭을 가로채지 않게 한다 (`SelfHitTestInvisible`은 자식인 배경 Border가 클릭을 받는다)
  - **임시 테스트**: `ANPCCharacter::Interact`에서 첫 행을 `GetController<AElmPlayerController>()->GetPlayerHUDWidget()->SetDialogueLine(...)`로 띄운다
- 빌드: 새 `UCLASS`, `UPROPERTY`가 있으므로 에디터를 끄고 IDE에서 빌드
- 에디터 작업(사용자)
  - `Content/UI/Game/WBP_Dialogue`: 부모 클래스 `DialogueWidget`. 반투명 배경(Border) 안에 `mSpeakerText`, `mDialogueText`(Auto Wrap Text). 넘기기 안내 글자(예: `▼`)는 BP에만 둔다
  - `WBP_PlayerHUD`: `WBP_Dialogue`를 화면 아래 가운데(Anchor 하단 중앙)에 놓고 이름을 `mDialogueWidget`으로. BindWidget 이름이 다르면 WBP 컴파일 에러가 난다

| 확인 | 기대 결과 |
| --- | --- |
| PIE 시작 | 대화창이 보이지 않음 |
| TavernKeeper 앞에서 E | 화면 아래에 "선술집 주인 / 어서 오게, 카일! …" |
| 긴 대사 | 창 안에서 줄바꿈되고 넘치지 않음 |
| 대화창이 떠 있을 때 화면 클릭 | (아직 진행 기능 없음) 이동 / 공격이 그대로 됨 = 대화창이 클릭을 가로채지 않음 |

### Step 3: 대화 진행

- 작업
  - 에셋(사용자): `Content/Input/IA_DialogueNext`(Value Type Digital(bool)), `Content/Input/IMC_Dialogue`(E, Space Bar, Left Mouse Button → `IA_DialogueNext`)
  - [DefaultInput.h/.cpp](../Source/ElmChronicles/Input/DefaultInput.h): `mDialogueMappingContext` 멤버, 생성자에서 `IMC_Dialogue`를 찾고 `IA_DialogueNext`를 `"DialogueNext"`로 등록
  - [ElmPlayerController.h/.cpp](../Source/ElmChronicles/Player/ElmPlayerController.h)
    - `SetupInputComponent`: `DialogueNext` Started → `AdvanceDialogue`
    - `StartDialogue`: 첫 행이 `DT_Dialogue`에 있는지 먼저 확인(없으면 에러 로그 후 시작하지 않음) → 인벤토리 닫기 → 안내 문구 숨기기, `mInteractTarget` 비우기 → `SetDialogueInput(true)` → `ShowDialogueLine`
    - `AdvanceDialogue`: 현재 행의 `NextRow`가 None이거나 찾을 수 없으면 `EndDialogue`, 아니면 `mDialogueRow`를 바꾸고 `ShowDialogueLine`
    - `EndDialogue`: 대화 중이 아니면 바로 return(여러 곳에서 불려도 안전하게). 대화창 숨기기 → `SetDialogueInput(false)` → 상태 비우기
    - `SetDialogueInput`: `UEnhancedInputLocalPlayerSubsystem`에서 한쪽 IMC를 빼고 다른 쪽을 우선순위 0으로 넣는다
    - `UpdateInteractTarget`: 대화 중이면 맨 앞에서 return (Step 4에서 끊김 확인을 여기에 넣는다)
  - [NPCCharacter.cpp](../Source/ElmChronicles/Characters/NPCCharacter.cpp): Step 1~2의 임시 테스트 코드를 지우고 `Interact`가 `StartDialogue(this, mDialogueRow)`를 부른다. `ElmPlayerController.h` include
- 빌드: 헤더 변경 → 에디터를 끄고 IDE에서 빌드

| 확인 | 기대 결과 |
| --- | --- |
| TavernKeeper 앞에서 E | 대화창에 `Tavern_01`. E를 계속 누르고 있어도 다음 대사로 넘어가지 않음 |
| E / Space / 마우스 왼쪽 | 한 번에 한 줄씩 `Tavern_05`까지 |
| `Tavern_05`에서 한 번 더 | 대화창이 사라지고 이동, 공격, 카메라, 인벤토리(I)가 다시 됨. `[E] 대화하기`가 다시 뜸 |
| 대화 중 WASD, 마우스 이동, I | 아무 반응 없음 |
| 인벤토리를 연 채로 E | 인벤토리가 닫히고 대화 시작, 커서가 사라짐 |
| 대화 끝난 뒤 바로 E | 다시 처음부터 대화 |

### Step 4: 대화 중 처리

- 작업
  - [NPCCharacter.h/.cpp](../Source/ElmChronicles/Characters/NPCCharacter.h): `BeginTalk(APawn* Listener)` = `BrainComponent->PauseLogic` → `StopMovement` → `SetFocus(Listener)`. `EndTalk()` = `ClearFocus(EAIFocusPriority::Gameplay)` → `BrainComponent->ResumeLogic`. **죽었어도 `ResumeLogic`을 부른다** ([주의](#주의) 첫 줄)
  - `AElmPlayerController::StartDialogue`: `mDialogueNPC->BeginTalk(플레이어)`, 플레이어를 NPC 방향(Yaw)으로 회전. `EndDialogue`: `EndTalk()` (NPC가 사라졌으면 건너뜀)
  - `AElmPlayerController::UpdateInteractTarget`: 대화 중이면 플레이어가 죽었거나, NPC가 유효하지 않거나, `CanInteract`가 false일 때 `EndDialogue`
  - [PlayerCharacter.cpp](../Source/ElmChronicles/Characters/PlayerCharacter.cpp) `OnDamagedBy`: 컨트롤러의 `EndDialogue` 호출
- 빌드: 헤더 변경 → 에디터를 끄고 IDE에서 빌드
- 테스트용 배치: TavernKeeper 근처(시야 800 밖)에 MonsterSpawner(BoneGrunt, Loop). 원거리 몬스터(BoneRogue / BoneMage)도 하나

| 확인 | 기대 결과 |
| --- | --- |
| 옆이나 뒤에서 말 걸기 | 플레이어는 즉시 NPC를 보고, NPC는 플레이어 쪽으로 돈다 |
| 순찰 / 귀가 중인 NPC에게 말 걸기 | 그 자리에 멈춘다. 대화가 끝나면 하던 행동을 다시 한다 |
| 대화 중 몬스터가 NPC 시야에 들어옴 | 0.1초 안에 대화가 끝나고 NPC가 몬스터와 싸운다 (공격 시 몬스터를 바라봄 = 포커스가 지워지지 않음) |
| 대화 중 플레이어가 투사체에 맞음 | 대화가 끝나고 조작이 돌아온다 |
| 대화 중 TavernKeeper가 쓰러짐 | 대화가 끝남 → 10초 뒤 일어나서 다시 싸우거나 귀가함. **가만히 서 있으면 `EndTalk`가 `ResumeLogic`을 건너뛴 것** |
| 대화 중 플레이어 사망 | 대화창이 사라짐 |
| 일반 대화(전투 없음) 회귀 | Step 3 테스트가 그대로 통과 |

## 에셋 작업

| 에셋 | 할 일 | 단계 | 담당 |
| --- | --- | --- | --- |
| `DataSource/DT_Dialogue.csv` | 스크립트로 생성 (5행) | 1 | Claude |
| `Content/Data/DT_Dialogue` | CSV 임포트, Row Type `DialogueTableRow` | 1 | 사용자 |
| `Content/Data/DA_GameData` | `mTables`에 `DT_Dialogue` 추가 | 1 | 사용자 |
| `Content/UI/Game/WBP_Dialogue` | 새로 만들기, 부모 `DialogueWidget` | 2 | 사용자 |
| `Content/UI/Game/WBP_PlayerHUD` | `mDialogueWidget` 배치 (하단 중앙) | 2 | 사용자 |
| `Content/Input/IA_DialogueNext` | 새로 만들기 (Digital) | 3 | 사용자 |
| `Content/Input/IMC_Dialogue` | E / Space Bar / Left Mouse Button → `IA_DialogueNext` | 3 | 사용자 |

## 진행 방식

- Claude는 코드를 직접 고치지 않는다. 단계마다 그대로 입력할 수 있는 전체 코드, 정확한 위치(파일, 함수 전체 교체 / 몇 번 줄 아래 등), 초보자용 설명(비유 포함), 테스트 표를 안내한다. 코드 주석은 "~한다" 체로 쓴다.
- 사용자가 "Step N 완료"라고 하면 `git diff`와 파일로 리뷰하고, 통과하면 다음 단계를 안내한다. 테스트와 커밋은 사용자가 한다.
- CSV(`DataSource/*.csv`)는 Claude가 `dt-csv-sync` 스크립트로 만들고 고친다. DT 임포트, WBP, IA / IMC 같은 에디터 에셋 작업은 사용자가 한다.
- 이 작업의 모든 단계에 헤더 변경이 있으므로, 빌드는 에디터를 끄고 IDE에서 한다.
- 단계가 리뷰를 통과하면 묻지 않고 이 문서의 상태를 ☑로 바꾼다. 작업이 모두 끝나면 이 문서를 지울지 묻는다.

## 이후 작업 (이 계획의 범위 밖)

- 퀘스트 연결: 대화가 끝날 때 이벤트를 내보내서 퀘스트 1(하몬 왕에게 작별 인사 → 5000골드) 완료, 퀘스트 3(선술집 의뢰) 수락에 쓴다. 행에 퀘스트 / 이벤트 열을 추가하는 방식이 유력하다.
- 선택지(예 / 아니오), 진행 상태에 따라 다른 시작 행(퀘스트 완료 뒤 대사 바꾸기)
- 타자기 효과(글자가 하나씩 나옴, 넘기기를 누르면 바로 전체 표시)
- 대화 중 카메라 연출, `Speaker`를 비우면 NPC 이름(DT_NPC `Name`)을 쓰기
- `FText`로 바꿔 현지화
- 미사용 `Content/Input/IA_Interaction` 정리
- 작업이 끝나면 대화 구조(`DT_Dialogue`, 대화 중 IMC 교체, NPC `BeginTalk` / `EndTalk`)를 CLAUDE.md의 "입력 / 상호작용 / UI"에 옮긴다
