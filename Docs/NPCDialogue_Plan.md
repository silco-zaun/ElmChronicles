# NPC 대화

> **다음 세션 시작 방법**: 이 문서를 읽고 → `git status` / `git diff`로 어디까지 했는지 확인하고 → [진행 상황](#진행-상황) 표에서 첫 번째 ☐ / ◐ 단계부터 이어서 한다.
>
> 마지막 갱신: 2026-10-30 17:23
>
> **지금 상태**: Step 3(상호작용 컴포넌트)과 Step 4(대화 컴포넌트) 코드가 모두 들어갔고 빌드도 성공했다. 하지만 두 빌드(16:41, 17:20) 모두 에디터를 켠 채 IDE에서 해서 Hot Reload로 들어갔고, 그 뒤 PIE를 한 번도 하지 않았다. **다음 할 일**: 에디터를 끄고(에셋 저장 안 함) 다시 연 뒤 Step 3 + Step 4 테스트 표 확인 → 사용자가 "Step 4 완료" → Claude 리뷰 → Step 5 안내. 아무것도 커밋하지 않았다([진행 방식](#진행-방식)의 커밋 나누기 참고).

## 목표

플레이어가 NPC 앞에서 F를 누르면 화면 아래에 대화창(말하는 사람 + 대사)이 뜨고, F / Space / 마우스 왼쪽으로 다음 대사로 넘긴다. 마지막 대사에서 넘기면 대화가 끝나고 원래 조작으로 돌아온다. 대화 중에는 이동, 공격, 카메라, 인벤토리가 막히고, NPC는 하던 행동을 멈추고 플레이어를 바라본다.

이 작업 안에서 상호작용과 대화 코드를 `AElmPlayerController`에서 컴포넌트 두 개(`UInteractionComponent`, `UDialogueComponent`)로 뺀다(2026-10-30 추가).

## 배경

상호작용까지는 이미 되어 있다. 이번 작업은 그 뒤를 잇는다.

| 이미 있는 것 | 위치 |
| --- | --- |
| 상호작용 인터페이스 `IInteractable` (`GetInteractText` / `CanInteract` / `Interact`) | [Interaction/Interactable.h](../Source/ElmChronicles/Interaction/Interactable.h). 구현: `ANPCCharacter`, `ALevelPortal` |
| 0.1초마다 주변 대상 검색, 상호작용 키(`IA_Interact`, F) → `Interact` 호출 | 지금은 `AElmPlayerController::UpdateInteractTarget` / `InteractKey`. Step 3에서 `UInteractionComponent`로 옮긴다 |
| 안내 문구 `[F] 대화하기` | `UPlayerHUDWidget::SetInteractPrompt`. 키 글자는 `UElmInputConfig::FindMappedKeyText`가 지금 켜진 IMC에서 `IA_Interact`에 매핑된 키를 물어 붙인다(2026-10-30 사용자 추가) |
| 대화용 입력 | `Content/Input/Dialogue/IA_DialogueNext`, `IMC_Dialogue`, `UDialogueInput`([Input/DialogueInput.h](../Source/ElmChronicles/Input/DialogueInput.h)). 기존 IA / IMC는 `Content/Input/Default/`로 옮겼다(2026-10-30 사용자) |
| NPC의 대화 시작 행 | `FNPCCharacterTableRow::DialogueRow` → `ANPCCharacter::mDialogueRow` (`InitAttributes`에서 읽음). `DT_NPC`의 `TavernKeeper` 행은 `Tavern_01`, `Villager`는 None |
| 대화 가능 조건 | `ANPCCharacter::CanInteract`: `mDialogueRow`가 있고, 살아 있고, BB `Target`이 비어 있음(전투 중이 아님) |

`ANPCCharacter::Interact`는 지금 Step 2의 임시 코드(첫 줄을 HUD에 바로 띄움)다.

리팩터링 배경(2026-10-30): 상호작용 기능이 늘 때마다 `AElmPlayerController`에 코드가 쌓였다(HUD / 창, 상호작용 검색, 대화, 입력 바인딩). 처음 계획의 Step 3(대화 진행) 코드를 컨트롤러에 반쯤 넣은 상태에서 멈추고, 상호작용과 대화를 컴포넌트로 빼기로 했다.

이 작업의 처음 설계는 삭제된 NPC 진행 문서의 "Step 5 계획"에 있다: `git show aca5af2^:./Docs/NPC_Progress.md`.

## 결정 사항

| 항목 | 결정 | 이유 / 버린 안 |
| --- | --- | --- |
| 코드 위치 | 상호작용 = `UInteractionComponent`, 대화 = `UDialogueComponent`. 둘 다 `AElmPlayerController`에 붙는 ActorComponent이고 파일은 `Source/ElmChronicles/Player/` (2026-10-30 사용자 결정. 폴더는 `UInventoryComponent`와 같은 곳) | 컨트롤러에는 HUD, 창(인벤토리 / 메뉴), `RefreshInputMode`, 키 바인딩만 남는다. 처음 계획은 "대화 상태는 컨트롤러가 든다. 별도 컴포넌트는 HUD / 입력 모드 / 상호작용 대상을 서로 찾아다녀야 해서 버림"이었다. 컨트롤러에 붙이면 `GetOwner<AElmPlayerController>()` 한 번으로 닿아서 그 문제가 없다. 버린 안: 캐릭터에 붙이기(HUD / IMC / 바인딩이 컨트롤러 쪽이고, 캐릭터는 바뀔 수 있는 몸이다. ASC를 PlayerState에 둔 것과 같은 이유), 일반 UObject(BeginPlay / EndPlay 수명을 직접 관리), 서브시스템(LocalPlayerSubsystem은 레벨 이동 뒤에도 남아 대화 상태가 넘어간다), GAS 어빌리티(Lyra `GA_Interact`, 이 규모에 과함), `Interaction/` 폴더(Claude가 처음 제안, 사용자가 `Player/`로 바꿈) |
| 상호작용 막기 | `UInteractionComponent::SetBlocked(bool)`. true면 대상 비우기 + 안내 문구 숨기기 + 검색과 F 무시. 대화가 시작 / 끝에서 부른다 | 상호작용 컴포넌트는 대화를 모른다. 나중에 상점 창, 컷신도 같은 함수로 막는다. 버린 안: `UpdateInteractTarget`이 `IsInDialogue()`를 묻기(Step 3 전 코드) |
| 키 바인딩 | 컨트롤러 `SetupInputComponent`에 그대로 둔다. 핸들러만 컴포넌트 함수(`BindAction(..., mInteraction.Get(), &UInteractionComponent::InteractKey)`) | 키를 한곳에서 본다. CLAUDE.md의 "UI / 상호작용 바인딩은 `AElmPlayerController::SetupInputComponent`" 규칙 유지. 컴포넌트가 스스로 바인딩하면 컨트롤러 `InputComponent`가 만들어지는 시점을 맞춰야 한다 |
| 컴포넌트 찾기 | 컨트롤러 getter(`GetInteraction()`, `GetDialogue()`, Step 4에서 추가). NPC는 `Player->GetController<AElmPlayerController>()->GetDialogue()` | `GetPlayerHUDWidget()`과 같은 방식. 버린 안: `FindComponentByClass` |
| 이름 | 옮기는 멤버 / 함수 이름은 그대로 둔다(`mInteractTarget`, `mInteractTimer`, `UpdateInteractTarget`, `InteractKey`, `mDialogueNPC`, `mDialogueRow`, `StartDialogue`, `AdvanceDialogue`, …) | 동작을 바꾸지 않는 이동이라 잘라 붙이기 쉽게. 입력 핸들러의 `~Key` 이름은 프로젝트 관례(`MoveKey`, `AttackKey`) |
| 대화용 입력 설정 | `UDialogueInput`(`UElmInputConfig` 자식)이 `IMC_Dialogue`(`mInputMappingContext`)와 `"DialogueNext"`를 든다 (2026-10-30 사용자) | 처음 계획은 `UDefaultInput::mDialogueMappingContext`였다. IMC 하나에 입력 설정 클래스 하나로 맞췄다 |
| 대화 데이터 | `DT_Dialogue`(`FDialogueTableRow`). 대사 한 줄이 한 행이고 `NextRow`로 다음 행을 잇는다. `NextRow`가 None이면 끝 | 행 이름이 `FName`이라 `FindRow`에 바로 넣을 수 있다 |
| 선택지 | 없음 (선형 대화) | Plan.md의 퀘스트는 모두 "말 걸고 의뢰 받기"라 선택지가 필요 없다. 필요해지면 행에 열을 추가한다 |
| 글자 타입 | `Speaker`, `Text`는 `FString` | 버린 안: `FText`. 다른 테이블(`Name`, `Description`)과 `dt-csv-sync` 스크립트가 `FString` 기준이다. 번역이 필요해지면 바꾼다 |
| `Speaker` | 대화창에 그대로 보일 이름을 적는다 ("선술집 주인", "카일") | NPC 이름(DT_NPC `Name`)과 자동 연결하지 않는다. 플레이어 대사도 같은 열로 처리하려고 |
| 상호작용 / 대화 넘기기 키 | F (2026-10-30 기획 변경. E는 기본 스킬, Q는 필살기가 된다. [Plan.md](Plan.md) "2. 조작") | 2026-10-30 확인: `IMC_Default`와 `IMC_Dialogue` 모두 F가 매핑되어 있다. 대화 중에는 `IMC_Default`가 빠지므로 E / Q 스킬도 함께 막힌다 |
| 대화 중 입력 | `IMC_Default`를 빼고 `IMC_Dialogue`(F / Space / 마우스 왼쪽 → `IA_DialogueNext`)를 넣는다 | 버린 안: `SetIgnoreMoveInput` 등으로 하나씩 막기. 공격, 점프, 카메라, 인벤토리를 각각 막아야 하고, 입력이 늘 때마다 빠뜨리기 쉽다 |
| 대화 중 메뉴(B) | 열리지 않는다. `IA_Menu`는 `IMC_Default`에 있으므로 대화 중에는 함께 빠진다. `IMC_Dialogue`에 B를 넣지 않는다 (2026-10-30, 메뉴 작업이 이 계획보다 늦게 들어와서 Claude가 기본값으로 정함) | 대화는 몇 줄이면 끝난다. 버린 안: `IMC_Dialogue`에도 B → `IA_Menu`. 일시정지 중에는 `IA_DialogueNext`(Trigger when Paused 꺼짐)가 들어오지 않아서 동작은 하지만, 메뉴의 귀환을 대화 중에 누르는 경우까지 확인해야 한다. 필요해지면 매핑만 추가하면 된다 |
| 커서 | 대화 중에는 커서를 띄우지 않는다. `IsAnyWindowOpen`에 대화창을 넣지 않는다 | 마우스 왼쪽 클릭으로 넘기므로 커서가 필요 없다. `IsAnyWindowOpen`이 true면 `RefreshInputMode`가 GameAndUI + 커서로 바꾼다 |
| 대사 찾기 | `FindDialogueLine(FName)`이 `DT_Dialogue`에서 행을 찾고, 찾은 행을 `UPlayerHUDWidget::SetDialogueLine`에 바로 넘긴다 | 처음 계획의 `ShowDialogueLine()`은 뺐다. HUD의 `SetDialogueLine`이 이미 "글자 넣고 보이기"를 하고, 시작 / 넘기기에서 행이 있는지 먼저 확인해야 해서 어차피 행을 찾는다 |
| 대화 시작 / 끝 정리 | 시작: 인벤토리 닫기(`SetInventoryOpen(false)`), 상호작용 막기(`SetBlocked(true)`). 끝: `SetBlocked(false)` | 막혀 있는 동안 검색이 멈추므로 대화창과 안내 문구가 같이 뜨지 않는다 |
| NPC 쪽 처리 | `ANPCCharacter::BeginTalk(Listener)`: BT `PauseLogic` → `StopMovement` → `SetFocus(Listener)`. `EndTalk()`: `ClearFocus(Gameplay)` → `ResumeLogic` | 버린 안: BT는 그대로 두고 `SetFocus`만 하기. 순찰(스포너의 `mPatrolPath`)이나 귀가(`HomeLocation`) 중이면 말하다가 걸어가 버린다. NPC 처리는 대화 컴포넌트가 아니라 NPC가 한다 |
| 플레이어 쪽 처리 | 대화를 시작할 때 NPC 방향으로 몸을 돌린다(Yaw만, `SetActorRotation`) | 플레이어는 `bOrientRotationToMovement`라서 입력이 없으면 스스로 돌지 않는다 |
| 대화가 끊기는 조건 | ① `UDialogueComponent`가 대화 중에만 도는 0.1초 타이머로 확인한다. 플레이어가 죽었거나 NPC가 사라졌거나 `CanInteract`가 false면 `EndDialogue`. ② 플레이어가 맞으면(`APlayerCharacter::OnDamagedBy`) `GetDialogue()->EndDialogue()` | `CanInteract`를 다시 쓰면 NPC가 쓰러지거나 전투에 들어가는 경우를 한 번에 처리한다. 멀리서 날아온 투사체처럼 NPC가 못 본 공격은 ②가 잡는다. 플레이어 사망은 `OnDamagedBy`가 아니라 `OnKilledBy`로 가므로 ①이 잡는다. 처음 계획은 ①을 `UpdateInteractTarget` 안에 넣는 것이었는데, 상호작용 컴포넌트가 대화를 모르게 하려고 옮겼다 |

## 주의

| 증상 | 원인 | 해결 |
| --- | --- | --- |
| 대화 중에 죽은 필수 NPC(TavernKeeper)가 부활한 뒤 가만히 서 있음 | `UBehaviorTreeComponent::StopTree`(사망 시 `OnKilledBy`가 부름)는 `bIsPaused`를 풀지 않는다. `ProcessExecutionRequest`는 `bIsPaused`면 실행을 무시하므로, `Revive`의 `RestartLogic` 뒤에도 BT가 멈춰 있다 (엔진 `BehaviorTreeComponent.cpp`의 `PauseLogic` / `ResumeLogic` / `ProcessExecutionRequest`에서 확인) | `EndTalk`는 NPC가 죽었어도 **반드시** `ResumeLogic`을 부른다. `IsDead()`로 건너뛰지 않는다 |
| (예방) 공격 태스크가 건 포커스가 지워짐 | `BTTask_MonsterAttack`도 `SetFocus(Target)`(Gameplay 우선순위)를 쓴다 | `EndTalk`에서 `ClearFocus` → `ResumeLogic` 순서를 지킨다. 전투로 대화가 끊길 때는 BT가 아직 멈춰 있어서 공격 태스크가 포커스를 걸기 전이다 |
| (예방) 컴포넌트 `BeginPlay`에서 HUD가 null | 컴포넌트의 `BeginPlay`는 컨트롤러 `BeginPlay`의 `Super::BeginPlay()` 안에서 불린다. HUD는 그 뒤에 컨트롤러가 만든다 | 컴포넌트 `BeginPlay`에서는 타이머만 걸고 HUD를 건드리지 않는다. 첫 검색은 0.1초 뒤라 그때는 HUD가 있다 |
| (예방) 컴포넌트 함수를 `BindAction`에 넘기면 컴파일 에러 | `BindAction`은 객체 포인터 타입(`UserClass*`)을 템플릿으로 추론한다. `TObjectPtr`를 그대로 넘기면 추론하지 못한다 | `mInteraction.Get()`처럼 원시 포인터로 넘긴다 |
| (예방) F로 대화를 시작하자마자 첫 대사가 넘어감 | F가 눌린 채로 IMC가 바뀐다 | `FModifyContextOptions`의 기본값 `bIgnoreAllPressedKeysUntilRelease = true`가 키를 뗄 때까지 무시해 준다. `AddMappingContext` / `RemoveMappingContext`에 옵션을 따로 넘기지 않는다 |
| (예방) 위 무시가 안 먹음: 대화 시작 F가 첫 줄을 넘기거나, 마지막 줄을 넘긴 F / Space / 클릭이 상호작용 / 점프 / 공격으로 이어짐 | 이 무시는 **Value Type이 Boolean인 액션에만** 걸린다(엔진 `EnhancedInputSubsystemInterface.cpp`의 `RebuildControlMappings`, `Mapping.Action->ValueType == EInputActionValueType::Boolean`). 2026-10-30 확인: `IA_Interact` / `IA_Jump` / `IA_Attack` / `IA_DialogueNext`는 Boolean이다 | `IA_DialogueNext`의 Value Type을 `Digital (bool)`에서 바꾸지 않는다 |
| (예방) 마지막 줄에서 `FindRow` 경고 로그 `requested invalid row 'None'` | `UDataTable::FindRow`는 `None`을 넣으면 경고를 남긴다 | `AdvanceDialogue`에서 `NextRow.IsNone()`을 먼저 보고, None이 아닐 때만 찾는다 |
| (예방) 새 IMC / IA를 코드가 못 찾음 (`CDO Constructor (DialogueInput): Failed to find`) | `UDefaultInput` / `UDialogueInput` CDO는 에디터(모듈)가 뜰 때 한 번 만들어진다. 그때 에셋 파일이 없으면 에디터를 다시 열 때까지 비어 있다 (`MenuUI_Progress.md`의 `IA_Menu`와 같은 함정). 에셋을 옮기면 경로도 바뀐다 | 에셋을 먼저 만들거나 옮겨서 저장 → 에디터를 끄고 빌드 → 에디터를 다시 연다 |
| 레벨 이동 때 대화용 IMC가 남지 않을까 | 적용된 IMC 목록(`AppliedInputContextData`)은 `UEnhancedPlayerInput`에 있고, 이것은 PlayerController가 가진다. `OpenLevel`이 컨트롤러를 새로 만들면 목록도 비고, 새 캐릭터 `BeginPlay`가 `IMC_Default`를 다시 넣는다 (엔진 `EnhancedPlayerInput.h`에서 확인). 컨트롤러에 붙은 두 컴포넌트도 함께 새로 만들어진다 | `EndPlay`에서 IMC나 대화 상태를 되돌릴 필요가 없다 |
| Hot Reload 뒤 PIE에 안내 문구도 대화창도 HUD도 없음 | 로그 `WBP_PlayerHUD_C is not a child class of Class /Script/ElmChronicles.PlayerHUDWidget`, `CDO Constructor (ElmPlayerController): Failed to find /Game/UI/Game/WBP_PlayerHUD.WBP_PlayerHUD_C`. Hot Reload가 컨트롤러 / HUD 클래스를 바꿔 끼우는 순간 컨트롤러 CDO가 WBP 클래스를 못 찾아 `mPlayerHUDWidgetClass`가 빈다. 2026-10-30 16:41 Step 3 빌드에서 실제로 났다(메뉴 작업 `MenuUI_Progress.md`에서도 같은 일). 17:20 Step 4 빌드도 같은 세션에서 Hot Reload로 들어갔다(그때는 이 에러 없음, `WBP_Dialogue` 다시 컴파일) | 코드가 틀린 것이 아니다. 그 세션에서는 에셋(특히 `WBP_PlayerHUD`, `WBP_Dialogue`)을 저장하지 말고 에디터를 끈다. 다시 열면 `UnrealEditor.modules`가 가리키는 마지막 DLL을 처음부터 불러와서 정상이다. 다음부터는 에디터를 끄고 빌드한다 |
| 로그 `CDO Constructor (ItemBox): Failed to find .../SM_PP_Theme_04_Spellbook_003` | `Item/ItemBox.cpp` 생성자가 찾는 메시 에셋이 없다. 2026-10-30 12:20 에디터 시작 때부터 매번 난다 | **이 작업과 관계없다.** 대화 / 상호작용 문제를 찾을 때 헷갈리지 않는다 |
| VS에서 빌드했는데 바뀐 코드가 없음 / Hot Reload 크래시 | Live Coding이 켜진 에디터는 UBT 빌드를 거부하고(출력 창에만 표시), Live Coding을 끈 에디터는 Hot Reload로 받는다 (CLAUDE.md "빌드 / 실행", 2026-10-30 메뉴 작업에서 둘 다 실제로 났다) | 헤더를 바꾼 단계는 항상 에디터를 끄고 빌드한다. 반영 여부는 `Binaries/Win64/UnrealEditor-ElmChronicles*.dll` 수정 시간으로 본다 |
| DT 임포트에서 Row Type 목록이 이상하거나 임포트가 깨짐 | 에디터를 켠 채로 IDE에서 빌드하면 Hot Reload가 일어나 `HOTRELOAD_` 구조체가 생긴다 | 새 `USTRUCT` / `UCLASS` / `UPROPERTY`가 있는 단계는 에디터를 끄고 빌드한다 |
| `Content/Input/Default/IA_Interaction`과 `IA_Interact`가 둘 다 있음 | `IMC_Default`가 참조하는 것은 `IA_Interact`다. `IA_Interaction`은 어디서도 참조하지 않는 잔재다 | 헷갈리지 않는다. 지우는 것은 사용자 판단 |
| CSV를 Write / Edit로 고치면 글자가 깨짐 | CSV가 UTF-16 LE + BOM이다 | `dt-csv-sync` 스크립트로만 만들고 고친다 |

## 목표 구조

```
AElmPlayerController (Player/)         HUD, 인벤토리 / 메뉴, RefreshInputMode, 모든 키 바인딩
 ├ mInteraction : UInteractionComponent (Player/InteractionComponent.h)
 │    BeginPlay: 로컬 컨트롤러면 0.1초 타이머 → UpdateInteractTarget → mInteractTarget, HUD 안내 문구
 │    InteractKey (IA_Interact, F)  → IInteractable::Interact
 │    SetBlocked(bool)
 └ mDialogue : UDialogueComponent (Player/DialogueComponent.h)
      StartDialogue / AdvanceDialogue (IA_DialogueNext) / EndDialogue / IsInDialogue

F → UInteractionComponent::InteractKey → ANPCCharacter::Interact
     → PC->GetDialogue()->StartDialogue(this, mDialogueRow)
          ├ 첫 행 확인 (없으면 에러 로그, 시작 안 함)
          ├ NPC->BeginTalk(플레이어)                     (Step 5)
          ├ 플레이어를 NPC 쪽으로 회전                    (Step 5)
          ├ PC->SetInventoryOpen(false), PC->GetInteraction()->SetBlocked(true)
          ├ SetDialogueInput(true)                        IMC_Default → IMC_Dialogue
          └ HUD->SetDialogueLine(첫 행)
AdvanceDialogue: NextRow가 있으면 그 행을 SetDialogueLine, None이면 EndDialogue
EndDialogue: NPC->EndTalk() (Step 5), 상태 비우기, 대화창 숨기기, IMC 되돌리기, SetBlocked(false)
```

```cpp
// Player/InteractionComponent.h (Step 3)
UCLASS() class UInteractionComponent : public UActorComponent
{
	TWeakObjectPtr<AActor> mInteractTarget;
	FTimerHandle mInteractTimer;
	bool mIsBlocked = false;
	static constexpr float InteractRadius = 200.f;
	static constexpr float InteractScanInterval = 0.1f;
	virtual void BeginPlay() override;
public:
	void InteractKey();
	void SetBlocked(bool Blocked);
private:
	void UpdateInteractTarget();
};

// Player/DialogueComponent.h (Step 4)
UCLASS() class UDialogueComponent : public UActorComponent
{
	TWeakObjectPtr<class ANPCCharacter> mDialogueNPC;
	FName mDialogueRow;          // None이면 대화 중이 아니다 (NPC가 사라져도 대화 중으로 남아야 끊을 수 있다)
	FTimerHandle mCheckTimer;    // Step 5: 대화 중에만 0.1초마다 끊김 확인
public:
	void StartDialogue(ANPCCharacter* NPC, FName StartRow);
	void AdvanceDialogue();
	void EndDialogue();
	bool IsInDialogue() const { return !mDialogueRow.IsNone(); }
private:
	const FDialogueTableRow* FindDialogueLine(FName Row) const;
	void SetDialogueInput(bool Dialogue);
	void CheckInterrupt();       // Step 5
};

// AElmPlayerController
UPROPERTY(VisibleAnywhere) TObjectPtr<UInteractionComponent> mInteraction;
UPROPERTY(VisibleAnywhere) TObjectPtr<UDialogueComponent> mDialogue;   // Step 4
UInteractionComponent* GetInteraction() const;                         // Step 4
UDialogueComponent* GetDialogue() const;                               // Step 4

// Data/Tables/DialogueTableRow.h (Step 1, 헤더만)
struct FDialogueTableRow : public FTableRowBase { FString Speaker; FString Text; FName NextRow; };

// UI/Game/DialogueWidget.h (Step 2): mSpeakerText, mDialogueText (BindWidget), SetLine
// UPlayerHUDWidget (Step 2): mDialogueWidget (BindWidget), SetDialogueLine, SetDialogueVisible

// Input/DialogueInput.h (사용자 추가): mInputMappingContext = IMC_Dialogue, "DialogueNext" = IA_DialogueNext

// ANPCCharacter (Step 5)
void BeginTalk(APawn* Listener);
void EndTalk();
```

## 진행 상황

| 단계 | 내용 | 상태 |
| --- | --- | --- |
| Step 1 | 대화 데이터: `FDialogueTableRow`, `DT_Dialogue.csv`, DT 임포트, 로그로 확인 | ☑ 코드 리뷰 통과, 빌드 확인, CSV 생성 (2026-10-27). DT 임포트 / DA 등록은 2026-10-30 파일로 확인 |
| Step 2 | 대화창 UI: `UDialogueWidget`, `WBP_Dialogue`, HUD 연결 | ☑ 2026-10-30 리뷰 통과. 커밋 `7d86c3e`에 코드와 에셋이 있다(`WBP_Dialogue`에 `mSpeakerText` / `mDialogueText`, `WBP_PlayerHUD`에 `mDialogueWidget`). 로그에 바인딩 에러 없음. PIE 화면 확인 기록은 없다 |
| Step 3 | 상호작용 컴포넌트: 컨트롤러의 상호작용 코드를 `UInteractionComponent`로 옮긴다 (리팩터링, 동작 변화 없음) | ◐ **코드 리뷰 통과(2026-10-30), PIE 테스트 남음.** 빌드는 Hot Reload로만 들어갔다([Step 3 리뷰 결과](#step-3-상호작용-컴포넌트)). CLAUDE.md "입력 / 상호작용 / UI"는 리뷰 통과 뒤 고쳤다 |
| Step 4 | 대화 컴포넌트 + 대화 진행: `UDialogueComponent`, NPC `Interact` 연결 (처음 계획의 Step 3) | ◐ **코드 입력 끝, 빌드 성공(17:20, Hot Reload), PIE 테스트와 정식 리뷰 남음.** 2026-10-30 17:23 Claude가 파일로 확인한 결과 안내와 같고, `NPCCharacter.cpp`에 쓰지 않는 include 두 줄만 남았다([Step 4 확인](#step-4-대화-컴포넌트--대화-진행)) |
| Step 5 | 대화 중 처리: NPC `BeginTalk` / `EndTalk`, 마주 보기, 끊기는 조건 (처음 계획의 Step 4) | ☐ |

사전 조건: 레벨에 `NPCCharacter`를 놓고 Details의 **M Row Name**을 `TavernKeeper`로 정해 둔다.

### Step 1: 대화 데이터 (완료)

- 작업: `Source/ElmChronicles/Data/Tables/DialogueTableRow.h`(헤더만), `DataSource/DT_Dialogue.csv`(Claude가 `dt-csv-sync`로 생성), `DT_Dialogue` 임포트, `DA_GameData.mTables`에 등록. `ANPCCharacter::Interact`에 로그 임시 테스트(Step 2에서 대화창 임시 테스트로 바뀜)
- CSV 내용 (Plan.md 퀘스트 3: 선술집, 고블린 토벌 보상 1000골드 + 포션)

| 행 | Speaker | Text | NextRow |
| --- | --- | --- | --- |
| `Tavern_01` | 선술집 주인 | 어서 오게, 카일! 왕실 기사를 그만뒀다는 소문은 들었네. | `Tavern_02` |
| `Tavern_02` | 카일 | 이제 먹고살 일을 찾아야지. 들어온 의뢰가 있나? | `Tavern_03` |
| `Tavern_03` | 선술집 주인 | 마침 잘 왔네. 왕국 근처에 고블린 무리가 나타나 상인들을 괴롭힌다더군. | `Tavern_04` |
| `Tavern_04` | 선술집 주인 | 녀석들을 처치해 주면 1000골드에 포션도 넉넉히 얹어 주지. | `Tavern_05` |
| `Tavern_05` | 카일 | 맡겨 두게. 금방 다녀오지. | None |

| 로그 | 원인 |
| --- | --- |
| `DialogueTableRow 테이블이 등록되지 않았습니다` | `DA_GameData`에 `DT_Dialogue`가 없거나 저장 안 함 |
| `requested row 'Tavern_0X' not in DataTable` | 행 이름 오타. CSV와 `DT_NPC`의 `DialogueRow` 비교 |

### Step 2: 대화창 UI (완료)

- 작업: `UI/Game/DialogueWidget.h/.cpp`(`SetLine`), `UPlayerHUDWidget`의 `mDialogueWidget` / `SetDialogueLine` / `SetDialogueVisible`(보일 때 `HitTestInvisible`: `SelfHitTestInvisible`은 자식인 배경 Border가 클릭을 받는다). 에셋 `WBP_Dialogue`, `WBP_PlayerHUD`의 `mDialogueWidget`(하단 중앙). 임시 테스트: `ANPCCharacter::Interact`가 첫 행을 HUD에 띄운다

### Step 3: 상호작용 컴포넌트

- 작업
  - 새 파일 `Source/ElmChronicles/Player/InteractionComponent.h/.cpp`: 컨트롤러의 `mInteractTarget`, `mInteractTimer`, `InteractRadius`, `InteractScanInterval`, `UpdateInteractTarget`, `InteractKey`를 옮기고 `mIsBlocked` / `SetBlocked`를 더한다. 타이머는 컴포넌트 `BeginPlay`에서 건다(로컬 컨트롤러만)
  - [ElmPlayerController.h/.cpp](../Source/ElmChronicles/Player/ElmPlayerController.h): 위 멤버 / 함수 삭제, 생성자에서 `mInteraction` 생성, `BeginPlay`의 타이머 삭제, `"Interact"` 바인딩을 컴포넌트 함수로, `"DialogueNext"` 바인딩을 `UDialogueInput`에서 찾게 수정. 컨트롤러에 남은 대화 코드(Step 4에서 옮김)는 `mInteractTarget` / 안내 문구 대신 `mInteraction->SetBlocked(true / false)`
  - [Interaction/Interactable.h](../Source/ElmChronicles/Interaction/Interactable.h) 주석: 대상을 찾는 쪽을 `UInteractionComponent`로
- 빌드: 새 `UCLASS`, `UPROPERTY` → 에디터를 끄고 빌드 → 에디터를 다시 열기
- 테스트 (대화는 아직 Step 2 임시 동작이다)

| 확인 | 기대 결과 |
| --- | --- |
| PIE 중 Outliner에서 플레이어 컨트롤러 선택 | Details에 `InteractionComponent`가 보인다 |
| TavernKeeper 앞 | `[F] 대화하기` (전과 같음) |
| 포탈 앞에서 F | 포탈 문구가 뜨고, F로 이동한다 |
| 도착 맵 | 안내 문구가 다시 뜬다 (새 컨트롤러의 컴포넌트가 타이머를 다시 건다) |
| TavernKeeper 앞에서 F | 대화창에 첫 줄 (Step 2 임시 동작) |
| Villager 앞 / NPC에게서 멀어짐 | 문구가 뜨지 않음 / 0.1초 안에 사라짐 |
| 죽은 직후 F | 아무 일도 없음 |

실패하면: 문구가 아예 안 뜸 → 생성자의 `CreateDefaultSubobject`, 컴포넌트 `BeginPlay`의 타이머(로컬 컨트롤러 확인). F가 안 먹음 → `"Interact"` 바인딩이 `mInteraction.Get()`, `&UInteractionComponent::InteractKey`인지.

Step 4 코드가 이미 들어가서 "TavernKeeper 앞에서 F → 첫 줄(Step 2 임시 동작)" 행은 Step 4 테스트 표로 대신한다. 나머지 행(Details, 문구, 포탈, 도착 맵, 사망 직후)은 그대로 확인한다.

2026-10-30 리뷰 결과:
- 코드(`Player/InteractionComponent.h/.cpp`, `ElmPlayerController.h/.cpp`, `Interactable.h` 주석)가 안내와 같다. 사용자가 생성자의 주석 처리된 `//bShowMouseCursor = true;`를 지웠다(문제없음). `Interactable.h`의 "E로" 주석은 사용자가 Step 4 때 "[상호작용 키]"처럼 키 이름을 빼는 쪽으로 고쳤다.
- 16:41 IDE 빌드는 `Result: Succeeded`였지만 에디터(12:16에 연 세션)가 켜진 채라 `Starting Hot-Reload from IDE`(`-0002.dll`)로 들어갔다. 그때 `WBP_PlayerHUD_C is not a child class of ...PlayerHUDWidget`, `CDO Constructor (ElmPlayerController): Failed to find .../WBP_PlayerHUD_C`가 났다([주의](#주의)). 마지막 PIE는 14:44(이 코드 전)라서 테스트 표는 아직 확인하지 않은 것으로 둔다.

### Step 4: 대화 컴포넌트 + 대화 진행

- 작업
  - 에셋은 다 있다(`Content/Input/Dialogue/`). 없으면 [주의](#주의)의 CDO 항목 순서대로 만든다
  - 새 파일 `Source/ElmChronicles/Player/DialogueComponent.h/.cpp`: 컨트롤러의 `mDialogueNPC`, `mDialogueRow`, `StartDialogue`, `AdvanceDialogue`, `EndDialogue`, `IsInDialogue`, `FindDialogueLine`, `SetDialogueInput`을 옮긴다. HUD / 인벤토리는 `GetOwner<AElmPlayerController>()`로, 상호작용 막기는 `GetInteraction()->SetBlocked`로, IMC는 owner의 `GetLocalPlayer()`로
    - `StartDialogue`: 이미 대화 중이면 무시. 첫 행이 `DT_Dialogue`에 있는지 먼저 확인(없으면 에러 로그 후 시작하지 않음) → 상태 저장 → 인벤토리 닫기 → `SetBlocked(true)` → `SetDialogueInput(true)` → `SetDialogueLine(첫 행)`
    - `AdvanceDialogue`: 현재 행의 `NextRow`가 None이거나 찾을 수 없으면 `EndDialogue`, 아니면 `mDialogueRow`를 바꾸고 `SetDialogueLine`
    - `EndDialogue`: 대화 중이 아니면 바로 return(여러 곳에서 불려도 안전하게). 상태 비우기 → 대화창 숨기기 → `SetDialogueInput(false)` → `SetBlocked(false)`
  - [ElmPlayerController.h/.cpp](../Source/ElmChronicles/Player/ElmPlayerController.h): 대화 코드 삭제, 생성자에서 `mDialogue` 생성, `GetInteraction()` / `GetDialogue()` getter, `"DialogueNext"` 바인딩을 `mDialogue.Get(), &UDialogueComponent::AdvanceDialogue`로. 대화 코드에만 쓰던 include 정리
  - [NPCCharacter.cpp](../Source/ElmChronicles/Characters/NPCCharacter.cpp): Step 1~2의 임시 테스트 코드를 지우고 `Interact`가 `GetDialogue()->StartDialogue(this, mDialogueRow)`를 부른다. 임시 코드에만 쓰던 `DialogueTableRow.h`, `PlayerHUDWidget.h` include를 뺀다
  - [UI/Game/DialogueWidget.h](../Source/ElmChronicles/UI/Game/DialogueWidget.h) 주석: 어떤 줄을 보여 줄지 정하는 쪽을 `UDialogueComponent`로
- 빌드: 새 `UCLASS` → 에디터를 끄고 빌드

| 확인 | 기대 결과 |
| --- | --- |
| TavernKeeper 앞에서 F | 대화창에 `Tavern_01`. F를 계속 누르고 있어도 다음 대사로 넘어가지 않음 |
| F / Space / 마우스 왼쪽 | 한 번에 한 줄씩 `Tavern_05`까지 |
| `Tavern_05`에서 한 번 더 | 대화창이 사라지고 이동, 공격, 카메라, 인벤토리(I)가 다시 됨. `[F] 대화하기`가 다시 뜸 |
| 대화 중 WASD, 마우스 이동, I, B | 아무 반응 없음 |
| 대화 중 | `[F] 대화하기` 문구가 보이지 않음 |
| 인벤토리를 연 채로 F | 인벤토리가 닫히고 대화 시작, 커서가 사라짐 |
| 대화 끝난 뒤 F를 뗐다가 다시 F | 다시 처음부터 대화 (`[F] 대화하기`가 다시 뜬 뒤. 마지막 줄을 넘긴 F를 누르고 있는 동안은 아무 일도 없음) |
| 마지막 줄을 Space / 마우스 왼쪽으로 넘김 | 점프 / 공격이 나가지 않음 |
| Output Log | `requested invalid row 'None'` 경고가 없음 |

실패하면: F로 대화가 안 열림 → Output Log의 `StartDialogue` 에러(행 이름), `CDO Constructor (DialogueInput): Failed to find`(에셋 경로). 대화창은 뜨는데 넘어가지 않음 → `"DialogueNext"` 바인딩, `IMC_Dialogue`의 키 매핑. 첫 줄이 바로 넘어감 / 마지막에 점프가 나감 → `IA_DialogueNext`의 Value Type. 대화가 끝났는데 움직이지 않음 → `SetDialogueInput(false)`에서 `IMC_Default`를 다시 넣는지. 대화 뒤 문구가 안 뜸 → `EndDialogue`의 `SetBlocked(false)`. HUD가 통째로 없음 → Hot Reload 세션([주의](#주의)).

2026-10-30 17:23 확인 (사용자가 "Step 4 완료"라고 하기 전에 진행 문서를 갱신하며 본 것. 정식 리뷰는 아직):
- 코드(`Player/DialogueComponent.h/.cpp`, `ElmPlayerController.h/.cpp`, `NPCCharacter.cpp`의 `Interact`, `DialogueWidget.h` 주석)가 안내와 같다. 컨트롤러는 HUD / 창 / `RefreshInputMode` / 키 바인딩만 남았고, include도 안내대로 정리됐다(`UObject/ConstructorHelpers.h`를 직접 include).
- 남은 것: `NPCCharacter.cpp`에 이제 쓰지 않는 `#include "../Data/Tables/DialogueTableRow.h"`, `#include "../UI/Game/PlayerHUDWidget.h"`가 남아 있다(빌드에는 지장 없음, 지우라고 안내했던 줄).
- 17:20 IDE 빌드는 `Result: Succeeded`, 다시 Hot Reload(`-0003.dll`, 같은 에디터 세션)로 들어갔다. 이번에는 `WBP_PlayerHUD_C` 에러는 없었고 `WBP_Dialogue`가 다시 컴파일됐다. 그 뒤 PIE 없음.
- 관계없는 기존 코드: `AElmPlayerController::SetMenuOpen` 주석에 "I / E는 막힌다"가 남아 있다(메뉴 작업 코드, 지금은 F).

### Step 5: 대화 중 처리

- 작업
  - [NPCCharacter.h/.cpp](../Source/ElmChronicles/Characters/NPCCharacter.h): `BeginTalk(APawn* Listener)` = `BrainComponent->PauseLogic` → `StopMovement` → `SetFocus(Listener)`. `EndTalk()` = `ClearFocus(EAIFocusPriority::Gameplay)` → `BrainComponent->ResumeLogic`. **죽었어도 `ResumeLogic`을 부른다** ([주의](#주의) 첫 줄)
  - `UDialogueComponent::StartDialogue`: `mDialogueNPC->BeginTalk(플레이어)`, 플레이어를 NPC 방향(Yaw)으로 회전, `mCheckTimer`(0.1초 반복) 시작. `EndDialogue`: 타이머 끄기, `EndTalk()` (NPC가 사라졌으면 건너뜀)
  - `UDialogueComponent::CheckInterrupt`: 플레이어가 죽었거나, NPC가 유효하지 않거나, `CanInteract`가 false면 `EndDialogue`
  - [PlayerCharacter.cpp](../Source/ElmChronicles/Characters/PlayerCharacter.cpp) `OnDamagedBy`: `GetController<AElmPlayerController>()->GetDialogue()->EndDialogue()`
- 빌드: 헤더 변경 → 에디터를 끄고 빌드
- 테스트용 배치: TavernKeeper 근처(시야 800 밖)에 MonsterSpawner(BoneGrunt, Loop). 원거리 몬스터(BoneRogue / BoneMage)도 하나

| 확인 | 기대 결과 |
| --- | --- |
| 옆이나 뒤에서 말 걸기 | 플레이어는 즉시 NPC를 보고, NPC는 플레이어 쪽으로 돈다 |
| 순찰 / 귀가 중인 NPC에게 말 걸기 | 그 자리에 멈춘다. 대화가 끝나면 하던 행동을 다시 한다 |
| 대화 중 몬스터가 NPC 시야에 들어옴 | 0.1초 안에 대화가 끝나고 NPC가 몬스터와 싸운다 (공격 시 몬스터를 바라봄 = 포커스가 지워지지 않음) |
| 대화 중 플레이어가 투사체에 맞음 | 대화가 끝나고 조작이 돌아온다 |
| 대화 중 TavernKeeper가 쓰러짐 | 대화가 끝남 → 10초 뒤 일어나서 다시 싸우거나 귀가함. **가만히 서 있으면 `EndTalk`가 `ResumeLogic`을 건너뛴 것** |
| 대화 중 플레이어 사망 | 대화창이 사라짐 |
| 일반 대화(전투 없음) 회귀 | Step 4 테스트가 그대로 통과 |

## 에셋 작업

| 에셋 | 할 일 | 단계 | 담당 | 상태 |
| --- | --- | --- | --- | --- |
| `DataSource/DT_Dialogue.csv` | 스크립트로 생성 (5행) | 1 | Claude | ☑ |
| `Content/Data/DT_Dialogue` | CSV 임포트, Row Type `DialogueTableRow` | 1 | 사용자 | ☑ 파일에 `DialogueTableRow`, `Tavern_01`~`05`가 있다 |
| `Content/Data/DA_GameData` | `mTables`에 `DT_Dialogue` 추가 | 1 | 사용자 | ☑ 파일에 `DT_Dialogue` 참조가 있다 |
| `Content/UI/Game/WBP_Dialogue` | 새로 만들기, 부모 `DialogueWidget` | 2 | 사용자 | ☑ `mSpeakerText`, `mDialogueText`가 있다 |
| `Content/UI/Game/WBP_PlayerHUD` | `mDialogueWidget` 배치 (하단 중앙) | 2 | 사용자 | ☑ `mDialogueWidget`, `WBP_Dialogue` 참조가 있다 |
| `Content/Input/Default/*` | 기존 IA / IMC를 `Default/` 폴더로 옮김 | 4 | 사용자 | ☑ 2026-10-30. `DefaultInput.cpp` 경로도 바꿈 |
| `Content/Input/Default/IMC_Default` | `IA_Interact` 매핑 E → F | 4 | 사용자 | ☑ 파일에 키 이름 `F`가 있고 `E`는 없다 |
| `Content/Input/Dialogue/IA_DialogueNext` | Value Type Digital(bool), Trigger when Paused 끔 | 4 | 사용자 | ☑ Boolean, `bTriggerWhenPaused` 없음 |
| `Content/Input/Dialogue/IMC_Dialogue` | F / Space Bar / Left Mouse Button → `IA_DialogueNext` | 4 | 사용자 | ☑ 파일에 `F`, `SpaceBar`, `LeftMouseButton`, `IA_DialogueNext`가 있다 |

## 진행 방식

- Claude는 코드를 직접 고치지 않는다. 단계마다 그대로 입력할 수 있는 전체 코드, 정확한 위치(파일, 함수 전체 교체 / 몇 번 줄 아래 등), 초보자용 설명(비유 포함), 테스트 표를 안내한다. 코드 주석은 "~한다" 체로 쓴다.
- Step 3 / 4는 동작을 바꾸지 않는 이동이 중심이라 이름을 그대로 두고 잘라 붙이기 위주로 안내한다.
- 사용자가 "Step N 완료"라고 하면 `git diff`와 파일로 리뷰하고, 통과하면 다음 단계를 안내한다. 테스트와 커밋은 사용자가 한다.
- CSV(`DataSource/*.csv`)는 Claude가 `dt-csv-sync` 스크립트로 만들고 고친다. DT 임포트, WBP, IA / IMC 같은 에디터 에셋 작업은 사용자가 한다.
- 이 작업의 모든 단계에 헤더 변경이 있으므로, 빌드는 에디터를 끄고 IDE에서 한다.
- 단계가 리뷰를 통과하면 묻지 않고 이 문서의 상태를 ☑로 바꾼다. md 파일(이 문서, CLAUDE.md, 다른 Docs)의 수정도 묻지 않고 하고, 고친 내용은 답에서 짧게 알린다. 작업이 모두 끝나면 이 문서를 지울지 묻는다.
- CLAUDE.md "입력 / 상호작용 / UI"의 상호작용 / 입력 설정 설명은 Step 3 리뷰 뒤 컴포넌트 기준으로 고쳤다(2026-10-30). 대화 구조는 작업이 끝나면 옮긴다.
- 메뉴 작업(`MenuUI_Progress.md`)과 같은 파일(`ElmPlayerController`, `PlayerHUDWidget`)을 고친다. 메뉴 작업의 미커밋 변경이 있으면 이 단계 커밋에 섞이지 않게 한다.
- 커밋 나누기 (2026-10-30 17:23 미커밋 변경을 Claude가 분류한 것. 마지막 커밋은 `d7e5748`)

| 묶음 | 파일 |
| --- | --- |
| 이 작업 (입력 정리 + F 키 + 상호작용 / 대화 컴포넌트) | `Content/Input/` 전체(루트의 IA / IMC 삭제, `Default/`, `Dialogue/`), `Source/.../Input/`(`DefaultInput.cpp` 경로, `ElmInputConfig.*`의 `FindMappedKeyText`, 새 `DialogueInput.*`), `Player/InteractionComponent.*`, `Player/DialogueComponent.*`, `Player/ElmPlayerController.*`, `Characters/NPCCharacter.cpp`, `UI/Game/DialogueWidget.h`, `UI/Game/PlayerHUDWidget.*`, `Interaction/Interactable.h`, `Interaction/LevelPortal.h`(주석), `Docs/NPCDialogue_Plan.md` |
| 메뉴 작업 | `GameModes/ElmGameModeBase.*`, `Subsystem/SaveSubsystem.*`, `UI/Game/MenuWidget.*`, `Docs/MenuUI_Progress.md` |
| 레벨 / 콜리전 | `Content/Assets/KayKit/...`(문틀 / 성문 메시 3개), `Content/Maps/ElmKingdom/Field.umap` |
| 기획 / 기타 | `Docs/Plan.md`(조작 변경: 상호작용 F, 스킬 E / Q), `README.md`, `Docs/LevelTravel_Progress.md` 삭제(이미 staged) |
| 여러 묶음에 걸침 | `CLAUDE.md`: 빌드 절차, 문틀 콜리전, 메뉴, 입력 / 상호작용 설명이 섞여 있다. `git add -p`로 나누거나 한 묶음에 넣는다 |

## 이후 작업 (이 계획의 범위 밖)

- 퀘스트 연결: 대화가 끝날 때 이벤트를 내보내서 퀘스트 1(하몬 왕에게 작별 인사 → 5000골드) 완료, 퀘스트 3(선술집 의뢰) 수락에 쓴다. `UDialogueComponent`에 "대화 끝남" 델리게이트를 두고, 행에 퀘스트 / 이벤트 열을 추가하는 방식이 유력하다.
- 선택지(예 / 아니오), 진행 상태에 따라 다른 시작 행(퀘스트 완료 뒤 대사 바꾸기)
- 타자기 효과(글자가 하나씩 나옴, 넘기기를 누르면 바로 전체 표시)
- 대화 중 카메라 연출, `Speaker`를 비우면 NPC 이름(DT_NPC `Name`)을 쓰기
- `FText`로 바꿔 현지화
- IMC 교체가 대화 말고도 필요해지면(상점, 컷신) 입력 모드 관리를 대화 컴포넌트에서 따로 뺀다
- 미사용 `Content/Input/Default/IA_Interaction` 정리
- 작업이 끝나면 대화 구조(`UDialogueComponent`, `DT_Dialogue`, 대화 중 IMC 교체, `SetBlocked`, NPC `BeginTalk` / `EndTalk`)를 CLAUDE.md의 "입력 / 상호작용 / UI"에 옮긴다
