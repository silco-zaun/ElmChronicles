# 캐릭터 창 (장비칸 + 능력치 + 캐릭터 미리보기)

> **다음 세션 시작 방법**: 이 문서를 읽고 → `git status` / `git diff`로 어디까지 했는지 확인하고 → [진행 상황](#진행-상황) 표에서 첫 번째 ☐ / ◐ 단계부터 이어서 한다.

## 목표
C 키로 여는 캐릭터 창을 만든다. 가운데에는 Idle 자세의 캐릭터가 보이고, 그 둘레에 장비칸 3개(무기 / 갑옷 / 목걸이)가 있다. 탭으로 "장비"와 "능력치" 화면을 오간다. 인벤토리에서 장비를 끼고 벗는 일과 장비 능력치 적용도 이 작업에 들어간다.

## 배경
- 2026-11-05 이전에는 장비 개념이 없었다. `UInventoryComponent`는 가방만 들고 있었고, 장비 아이템을 우클릭해도 아무 일도 일어나지 않았다.
- 기획(`Docs/Plan.md` 8.1)의 장비칸은 무기 / 갑옷 / 목걸이 3칸이다. `DT_Item`에는 이미 Weapon / Armor / Necklace 아이템과 능력치 옵션(`Options`)이 있다.

## 결정 사항

| 항목 | 결정 | 이유 / 버린 안 |
|---|---|---|
| 미리보기 방식 | 보이지 않는 곳에 **미리보기 전용 액터**(메시 + 조명)를 두고 SceneCapture2D로 찍는다. 결과를 RenderTarget → 머티리얼 → UMG Image로 보여 준다 | 언제나 Idle이고 실제 플레이어와 상관없다. 버린 안: "실제 플레이어 촬영"은 창을 연 채 움직이거나 맞으면 미리보기도 따라 움직인다. "고정 초상화"는 3D가 아니다. |
| 장비칸 | 기획대로 3칸(무기 / 갑옷 / 목걸이). `EEquipSlot` 값 = 칸 번호 | 투구 / 장갑 / 신발 같은 칸은 데이터가 없다. 늘리려면 `EItemType`과 DT_Item도 함께 늘려야 한다. |
| 탭 | 장비 / 능력치 두 개 | 버린 안: 두 번째 탭을 비워 두고 나중에 스킬로 채우기. |
| 착용 / 해제 | 가방 칸 **우클릭 또는 더블클릭** = 착용(끼고 있던 것과 맞바꿈). 장비칸 **우클릭 또는 더블클릭** = 해제 | 원래는 우클릭만이었고, 2026-11-05 사용자가 더블클릭을 더했다. 버린 안: 가방 ↔ 장비칸 드래그. 창이 달라서 `NativeOnDrop`에서 칸 종류를 다루는 일이 늘어난다. |
| 장비 능력치 | 칸마다 Infinite GE(`UGameplayEffect_Equipment`, SetByCaller 4개) 하나. Handle은 `mEquipEffectList`에 둔다 | Base 값을 건드리지 않아서 레벨업(`InitLevelStats`)과 메모(Base 값만 적는다)가 섞이지 않는다. |
| 장비 겉모습 | **장비는 메시를 바꾸지 않는다.** 어떤 무기를 끼든 DT_Player `MainhandMesh`를 들고, 무기칸이 비면 그 메시를 숨긴다(`APlayerCharacter::SetWeaponVisibility`) | 2026-11-05 사용자 결정. `InitAsset`과 `InitInventory` 중 어느 쪽이 먼저 불려도 결과가 같다(아래 주의 1). |
| 아이템 줍기 | **없앴다.** 아이템은 가방 / 장비칸의 아이콘으로만 있고, 레벨 액터와 메시가 없다. `FItemTableRow::Mesh`와 `AItemBox`를 지웠다 | 2026-11-05 사용자 결정. 몬스터 드랍은 처치할 때 가방에 바로 넣는다(`AddItem`은 그대로 둔다). |
| 능력치 표시 | 장비 탭에는 장비가 바꾸는 네 값을 `공격력 30 (+10)`처럼 보여 준다. 괄호 안 값 = 현재 값 − Base 값. 능력치 탭에는 레벨, 경험치(`0 / 100`), 골드, HP / MP, 공격 속도, 이동 속도, 사거리가 있다 | 탭마다 TextBlock 하나에 여러 줄을 쓴다. 숫자마다 위젯을 두면 BindWidget이 14개로 늘어난다. |
| 선택된 탭 표시 | 지금 탭의 버튼을 `SetIsEnabled(false)`로 끈다(흐리게 그려진다) | 스타일을 따로 만들지 않아도 된다. |
| 창 이동 | **넣지 않았다.** 캐릭터 창은 화면 왼쪽 가운데에 고정된다 | 인벤토리의 타이틀바 드래그 코드(약 50줄)를 옮겨야 한다. 필요하면 나중에 한다. |
| 아이템 정보창 위치 | 장비칸 아이콘이면 캐릭터 창 **오른쪽**, 가방 아이콘이면 인벤토리 **왼쪽**에 붙인다(`ShowItemInfoWidget(bool, ESlotType)` → `PlaceItemInfoNextTo`) | 두 창이 화면 양쪽에 있어서, 정보창이 화면 안쪽으로 뜨게 한다. |

## 주의

1. **`InitInventory`와 `InitAsset`의 순서는 정해져 있지 않다.** 둘 다 `CallOrRegisterDataLoaded`로 데이터 로드를 기다리는데, 멀티캐스트 델리게이트는 등록한 순서의 반대로 부른다. 그래서 무기칸이 메시를 "바꾸게" 하면 나중에 불린 `InitAsset`이 덮어쓴다. 무기칸은 보이기 / 숨기기만 하고, 메시와 위치는 `InitAsset`만 정한다.
2. **새 게임에서는 장비칸을 따로 맞춰야 한다.** 무기 메시 컴포넌트는 처음에 보이는 상태다. 메모가 있을 때만 `RefreshEquipSlot`을 부르면 새 게임에서 빈손인데 칼이 보인다. `InitInventory` 끝에서 메모 여부와 상관없이 모든 칸에 `RefreshEquipSlot`을 부른다.
3. **레벨 이동 뒤 장비 GE는 다시 걸어야 한다.** `OpenLevel`이 PlayerState(ASC)를 새로 만들어서 GE가 사라진다. `ReadEquipFromSave`는 목록만 채우고, 위 2번의 반복문이 GE를 다시 건다.
4. **더블클릭의 두 번째 누름은 `NativeOnMouseButtonDown`으로 오지 않는다.** `NativeOnMouseButtonDoubleClick`으로만 온다(엔진 `FSlateApplication::ProcessMouseButtonDoubleClickEvent`). 오른쪽 버튼도 그렇다. 빠른 우클릭 두 번의 두 번째도 여기로 오므로, 더블클릭 함수가 왼쪽과 오른쪽을 모두 받아서 `UseSlotItem`을 부른다. 안 받으면 두 번째 우클릭이 사라진다.
5. **슬롯 번호가 두 가지다.** 가방 칸 번호(0~49)와 장비칸 번호(`EEquipSlot`, 0~2)는 다른 목록이다. `USlotWidget::NativeOnDrop`이 장비칸에 놓인 것을 `SwapItem`에 넘기면 엉뚱한 가방 칸끼리 바뀐다. 그래서 `mType != ESlotType::Inventory`이면 놓기를 받지 않는다. 장비칸 아이콘은 왼쪽 클릭에서 드래그 감지 없이 `Handled`로 끝낸다.
6. **경험치 표시에서 행을 너무 일찍 찾으면 로그가 남는다.** 데이터 로드 전이나 `InitAttributes` 전(Level 0)에 `FindRow<FLevelStatTableRow>`를 부르면 "테이블 없음 / 행 없음" 로그가 남는다. `Level > 0 && IsDataLoaded()`일 때만 찾는다. `RequiredExp`는 지금 레벨에서 다음 레벨까지 필요한 양이고, 0이면 최대 레벨이다.
7. **`GetOwningPlayerState()`를 `AActor*`로 넘기려면 `GameFramework/PlayerState.h`가 필요하다.** `CharacterInfoWidget.cpp`가 이 include를 갖고 있다(`StatusBarWidget.cpp`도 같다). 전방 선언만 있으면 `APlayerState*` → `const AActor*` 변환을 컴파일러가 모른다.
8. **에디터가 꺼지는 중에 빌드하면 `UnrealEditor-ElmChronicles-0001.dll`이 생긴다.** 2026-11-05 17:07 빌드가 그랬다. Hot Reload는 없었고, 모듈 목록이 `-0001`을 가리켜서 해롭지는 않았다. 작업 관리자에서 `UnrealEditor`가 사라진 것을 보고 빌드한다.
9. **WBP를 만드는 동안 나는 BindWidget 에러는 정상이다.** `A required widget binding "mXxx" ... was not found`는 그 이름의 위젯을 다 놓으면 사라진다. `WBP_PlayerHUD`도 `mCharacterInfoWidget`을 놓기 전까지 같은 에러가 난다.
10. **`IA_CharacterInfo`는 빌드 전에 있어야 한다.** `UDefaultInput` CDO가 에디터를 시작할 때 경로로 찾는다.

## 목표 구조

```cpp
// Player/InventoryComponent.h
enum class EEquipSlot : uint8 { Weapon, Armor, Necklace, Count UMETA(Hidden) };
TArray<TObjectPtr<UItemObject>> mEquipList;          // 칸 번호 = EEquipSlot
TArray<FActiveGameplayEffectHandle> mEquipEffectList; // 칸마다 장비 GE 하나
UItemObject* GetEquipItem(EEquipSlot Slot) const;
bool UseItem(int32 Index);            // 장비면 EquipItem(맞바꿈), 포션이면 UsePotion
bool UnequipItem(EEquipSlot Slot);    // 가방이 가득 차 있으면 false
void RefreshEquipSlot(EEquipSlot Slot); // GE 다시 걸기 + 무기 보이기 / 숨기기 + mOnEquipChanged
template <typename T> void AddEquipChangeCallback(T* Obj, void (T::*Func)(UItemObject*, int32));

// UI/UITypes.h
enum class ESlotType : uint8 { Inventory, Skill, Quick, Equipment };

// UI/Game/CharacterInfoWidget.h  (WBP_CharacterInfo의 부모)
// BindWidget: mCloseButton, mEquipTabButton, mStatTabButton, mTabSwitcher(0 = 장비, 1 = 능력치),
//             mWeaponSlot / mArmorSlot / mNecklaceSlot(WBP_Slot), mEquipStatText, mDetailStatText
// NativeConstruct에서 인벤토리(장비 변경)와 PlayerState ASC(Attribute 변경)를 구독하고, NativeDestruct에서 끊는다.

// UI/Game/PlayerHUDWidget.h
void ShowItemInfoWidget(bool IsShow = true, ESlotType From = ESlotType::Inventory);
void SetCharacterInfoVisible(bool Visible);
bool IsCharacterInfoVisible() const;   // IsAnyWindowOpen에 들어 있다

// Player/ElmPlayerController.h
void ToggleCharacterInfo();            // IA_CharacterInfo(C) Started
void SetCharacterInfoOpen(bool Open);  // HUD + RefreshInputMode. 대화 시작(UDialogueComponent)도 이것으로 닫는다
```

- 장비 GE: `AbilitySystem/Effects/GameplayEffect_Equipment.h/.cpp`, 태그 `ElmTags::SetByCaller_Equip_PhysicalAttack / MagicalAttack / PhysicalDefense / MagicalDefense`. 네 개를 모두 채운다(SetByCaller 규칙).
- 메모: `UElmSaveGame::Equipment`. `APlayerCharacter::WriteToSave`가 `mInventory->WriteToSave(Save.Inventory, Save.Equipment)`로 적는다.

## 진행 상황

| 단계 | 내용 | 상태 |
|---|---|---|
| 1 | 장비 로직: 착용 / 해제 / 맞바꿈, 장비 GE, 무기 보이기, 메모, 아이템 줍기 제거, 더블클릭 착용 | ☑ (2026-11-05 PIE 확인. 더블클릭은 2단계 확인 표에서 함께 본다) |
| 2 | 캐릭터 창: C 키, 탭, 장비칸 3개, 장비칸 해제, 능력치 표시, 정보창 위치 | ◐ 코드, 빌드(18:28), `WBP_CharacterInfo` / `WBP_PlayerHUD` 저장(18:44)까지 했다. PIE 확인이 남았다 |
| 3 | 가운데 캐릭터 미리보기 | ☐ |
| 4 | 마무리: CLAUDE.md 갱신, 커밋, 이 문서 정리 | ☐ |

### 1단계: 장비 로직 (완료)
- 작업: `InventoryComponent`(`mEquipList`, `EquipItem`, `UnequipItem`, `RefreshEquipSlot`, `ReadEquipFromSave`, `InitInventory` 끝 반복문), `GameplayEffect_Equipment`, `ElmGameplayTags`(Equip 태그 4개), `ElmSaveGame::Equipment`, `PlayerCharacter`(`SetWeaponVisibility`, `WriteToSave`), `ItemTableRow`(Mesh 삭제), `Item/ItemBox.*` 삭제, `IconWidget`(`UseSlotItem`, `NativeOnMouseButtonDoubleClick`).
- 데이터: `DataSource/DT_Item.csv`에서 `Mesh` 열을 지웠다. `DT_Player.csv` Kyle의 `MainhandMesh`는 `/Game/Assets/KayKit/Adventurers/Assets/sword_1handed.sword_1handed`다. 둘 다 사용자가 다시 임포트했다.

### 2단계: 캐릭터 창 (PIE 확인 남음)
- 작업(반영됨): `UITypes.h`(`Equipment`), `DefaultInput.cpp`(`CharacterInfo`), `CharacterInfoWidget.h/.cpp`(새 파일), `PlayerHUDWidget`(`mCharacterInfoWidget`, `PlaceItemInfoNextTo`, `IsAnyWindowOpen`), `ElmPlayerController`(토글 / 바인딩), `DialogueComponent.cpp`(대화 시작 때 닫기), `IconWidget`(`UseEquipItem`, `EquipItemRender`, 장비칸 드래그 막기), `SlotWidget.cpp`(`NativeOnDrop` 가방 칸만).
- 빌드: 에디터를 끈 뒤 CLAUDE.md의 `Build.bat` 명령.
- 테스트(시작 맵 `/Game/Maps/Test`, 새 게임 = 가방에 목검 / 천갑옷 / 체력 포션 1개):

| 확인 | 기대 결과 |
|---|---|
| C 키 / X 버튼 | C로 열면 커서가 보인다. C를 다시 누르거나 X를 누르면 닫히고 커서가 사라진다 |
| 탭 | 장비 ↔ 능력치가 바뀌고, 지금 탭의 버튼이 흐리게 보인다 |
| 처음 상태 | 장비칸 3개가 비어 있고, 손에 칼이 없고, `공격력 20`이다(Lv1) |
| 가방의 목검 **더블클릭** | 무기칸에 목검이 들어가고 `공격력 30 (+10)`, 손에 칼이 보인다. 실패하면 `NativeOnMouseButtonDoubleClick`을 의심한다 |
| 가방의 천갑옷 **우클릭** | `방어력 30 (+10)`, `마법 방어력 30 (+10)` |
| 무기칸 우클릭 / 더블클릭 | 목검이 가방 빈 칸으로 돌아가고, 칼이 사라지고, `공격력 20`이 된다 |
| 장비칸 아이콘에 마우스 올리기 | 정보창이 캐릭터 창 **오른쪽**에 뜬다. 가방 아이콘이면 예전처럼 인벤토리 왼쪽에 뜬다 |
| 가방 아이콘을 장비칸에 끌어다 놓기 | 아무것도 바뀌지 않는다. 장비칸 아이콘은 끌리지 않는다 |
| 능력치 탭 | `레벨 1`, `경험치 0 / 100`. 몬스터를 잡으면 경험치와 골드가 창을 연 채로 바로 바뀐다 |
| 포탈로 다른 맵에 다녀오기 | 장비칸, 손의 칼, `(+N)` 수치가 그대로다. 수치가 사라지면 주의 3을 의심한다 |
| NPC와 대화 시작 | 캐릭터 창이 닫힌다 |
| 로그 | `LogTestDebug` 경고(`ASC가 없어`, DT 행 없음)와 `LogHotReload`가 없다 |

- 보류: **포션 빠르게 우클릭 두 번 → 2개 소모** 확인. 새 게임 포션이 1개뿐이라(`GiveStartingItems`가 `AddItem(Key)`로 1개씩 준다) 지금은 할 수 없다. 포션이 2개 이상 생기면(상점이나 드랍) 확인한다.

### 3단계: 캐릭터 미리보기 (계획, 아직 검증하지 않음)
- 미리보기 액터(C++ 1개): SkeletalMesh(DT_Player 행의 메시 / ABP 또는 Idle 애니메이션) + 무기 메시 + 조명 + `USceneCaptureComponent2D`. 레벨에 보이지 않는 곳(맵 아래 등)에 스폰한다.
- 캡처:
  - `PrimitiveRenderMode = UseShowOnlyList`로 이 액터의 컴포넌트만 찍는다.
  - 배경을 비우려면 `CaptureSource = SceneColorHDR`로 찍고, 머티리얼에서 알파를 뒤집는다(1 − A).
  - 창이 닫혀 있으면 캡처를 끈다(`bCaptureEveryFrame` / `bCaptureOnMovement` 끄기).
- 표시: RenderTarget → User Interface 도메인 머티리얼 → `WBP_CharacterInfo`의 Image. 이 프로젝트는 Substrate라서 Blend Mode 이름이 다르다(Translucent = `TranslucentGreyTransmittance`).
- 정할 것:
  - 미리보기의 무기가 무기칸을 따라 보이고 숨는지(`mOnEquipChanged` 구독).
  - 마우스 드래그 회전을 넣을지.
- **WBP 준비가 필요하다.** 2단계 가이드는 장비 탭 가운데에 `PreviewArea`라는 Border를 두라고 했지만, 지금 `WBP_CharacterInfo`에는 그 이름의 위젯이 없다(2026-11-05 에셋 문자열로 확인). 3단계를 시작할 때 미리보기 Image를 놓을 자리를 사용자와 먼저 확인한다.

### 4단계: 마무리
- CLAUDE.md의 "입력 / 상호작용 / UI"에서 창 목록을 "창(인벤토리 I, 퀘스트 P, 캐릭터 C)"로 고친다. "인벤토리 / 장비" 섹션에 캐릭터 창 항목을 더한다(구독 방식, 장비칸 슬롯 번호 = `EEquipSlot`, 정보창 위치 규칙).
- 커밋은 사용자가 요청할 때 한다. 지금 작업 트리에 1~2단계 변경이 모두 커밋되지 않은 채 있다(마지막 커밋 `5a3df08`).
- 끝나면 이 문서를 지울지 사용자에게 묻는다.

## 에셋 작업

| 에셋 | 할 일 | 담당 | 상태 |
|---|---|---|---|
| `Content/Input/Default/IA_CharacterInfo` | `IA_Quest`를 복제해 만든다(Digital) | 사용자 | ☑ |
| `Content/Input/Default/IMC_Default` | `IA_CharacterInfo` → C 매핑 | 사용자 | ☑ |
| `Content/UI/Game/WBP_CharacterInfo` | 부모 `CharacterInfoWidget`. BindWidget 9개(이름 = C++ 변수, Is Variable). 버튼 3개는 Is Focusable 끔. Widget Switcher 자식 순서 0 = 장비, 1 = 능력치 | 사용자 | ☑ (PIE 확인 전) |
| `Content/UI/Game/WBP_PlayerHUD` | `mCharacterInfoWidget` 배치: 왼쪽 가운데 앵커, Position (40, 0), Alignment (0, 0.5), Size To Content. Hierarchy에서 `mItemInfoWidget` / `mCelebrationWidget` / `mMenuWidget` / `mConfirmWidget`보다 앞 | 사용자 | ☑ (PIE 확인 전) |
| `Content/Data/DT_Item` | `DataSource/DT_Item.csv`(Mesh 열 삭제)로 다시 임포트 | 사용자 | ☑ |
| `Content/Data/DT_Player` | `DataSource/DT_Player.csv`(Kyle 무기 메시)로 다시 임포트 | 사용자 | ☑ |
| 3단계 RenderTarget / 머티리얼 / 미리보기 액터 BP | 3단계 가이드에서 정한다 | 사용자 | ☐ |

## 진행 방식
- **Claude는 C++ 소스와 에셋을 직접 고치지 않는다.** 단계마다 따라 하는 가이드를 준다(준비 → 파일:위치 → 바꾸기 전 / 후 코드 → 빌드 → PIE 확인, 단계마다 짧은 "왜"). 사용자가 붙여 넣고 빌드하고 에디터 작업을 한다.
- 사용자가 "완료"라고 하면 Claude가 확인한다. 보는 것: `git diff`로 반영 상태, `Binaries/Win64/UnrealEditor-ElmChronicles*.dll` 시간과 `-000N.dll`이 있는지, `Saved/Logs/ElmChronicles.log`의 `LogHotReload` / `LogTestDebug` / BindWidget 에러.
- 새 소스 파일은 UTF-8(BOM)로 만든다. 기존 파일(예: `QuestWidget.h/.cpp`)을 탐색기에서 복사해 이름을 바꾸면 BOM이 유지된다.
- CSV는 `.claude/skills/dt-csv-sync/scripts/dt_csv.py`로만 고친다. Git Bash에서 `/Game/...` 값을 넘길 때는 `MSYS_NO_PATHCONV=1`을 붙인다(안 붙이면 `C:/Program Files/Git/Game/...`로 바뀐다).
- MD 문서(CLAUDE.md, Docs, README)는 Claude가 묻지 않고 고친다.
- 무거운 UI·입력 처리를 넣을 때는 비용을 먼저 알리고 단순한 대안을 함께 낸다.

## 이후 작업 (이 계획의 범위 밖)
- 캐릭터 창 타이틀바 드래그(인벤토리 창 코드 재사용).
- 몬스터 드랍 아이템: `DT_Monster`의 `DropItems`가 모두 비어 있다. 처치할 때 가방에 바로 넣는 규칙만 정해 두었다.
- 시작 아이템을 코드(`GiveStartingItems`, 임시)에서 데이터로 옮기기.
