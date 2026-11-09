# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

Unreal Engine 5.8 C++ 싱글플레이 액션 RPG. 런타임 모듈은 `ElmChronicles` 하나다. 기획(능력치, 데미지 공식, 몬스터 등급, 아이템, 퀘스트, 주차별 일정)은 [Docs/Plan.md](Docs/Plan.md)에 있고, 수치나 규칙을 정할 때 이 문서를 기준으로 한다. 핵심 기술은 AssetManager, GAS(GameplayAbilities), AIPerception + BehaviorTree다. 여러 세션에 걸친 작업의 계획·진행 문서는 `Docs/*_Plan.md` / `*_Progress.md`이고 `progress-doc` 스킬로 쓰고 이어받는다.

## 빌드 / 실행

```powershell
# 저장소 루트에서 실행한다. (PC마다 저장소 위치가 달라서 절대 경로를 적지 않는다)
# 에디터 타깃 빌드 (UBT가 Source 폴더를 직접 훑으므로 새 .h/.cpp도 .sln 재생성 없이 컴파일된다)
& "C:\UE_5.8\Engine\Build\BatchFiles\Build.bat" ElmChroniclesEditor Win64 Development -Project="$PWD\ElmChronicles.uproject" -WaitMutex

# 에디터 실행
& "C:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\ElmChronicles.uproject"
```

- 에디터가 Live Coding으로 열려 있으면 UBT 빌드가 거부된다(Visual Studio 빌드도 같다). 에디터를 닫고 빌드하거나, 에디터에서 Ctrl+Alt+F11로 Live Coding 컴파일한다. `UPROPERTY`/`UCLASS` 구조가 바뀐 헤더 변경은 에디터를 재시작해야 안전하다.
  - VS에서는 거부 메시지(`Unable to build while Live Coding is active`)가 출력 창에만 나와서 빌드된 줄 알기 쉽다(2026-10-30). 반영됐는지는 `Binaries/Win64/UnrealEditor-ElmChronicles*.dll` 수정 시간이나 `%LOCALAPPDATA%\UnrealBuildTool\Log.txt`의 `Result:`로 확인한다.
- Live Coding이 시작되지 않았거나 에디터에서 Live Coding을 끈 채(2026-10-30) 에디터가 열려 있으면 UBT가 빌드를 막지 않고 `UnrealEditor-ElmChronicles-0001.dll`을 만들며, 에디터가 Hot Reload를 시도한다. 클래스를 지운 변경에서는 이 Hot Reload 중에 에디터가 크래시했다(2026-10-27). IDE 빌드는 항상 에디터를 끄고 한다.
  - 새 행 구조체를 Hot Reload로 받은 세션에서 임포트한 DT는, 에디터를 다시 열면 `Missing RowStruct while loading DataTable`이 나며 행을 잃는다(2026-11-02 `DT_Quest`. DT를 지우고 다시 임포트해서 고쳤다). 새 `USTRUCT`가 들어간 빌드 뒤에는 에디터를 다시 연 다음에 임포트한다.
- `.sln`/`.slnx`는 gitignore 대상(엔진이 생성)이다.
- 자동화 테스트나 린트 설정은 없다. 검증은 PIE로 한다. 시작 맵은 `/Game/Maps/ElmKingdom/Field`다(`DefaultEngine.ini` `GameDefaultMap` / `EditorStartupMap`).
- 패키지(Platforms → Package Project): Shipping은 로그 파일을 남기지 않는다. 패키지에서만 생기는 문제는 Development로 패키지해서 `<패키지 폴더>/ElmChronicles/Saved/Logs/ElmChronicles.log`를 본다. 쿠킹은 Zen 저장소에 들어가서 `Saved/Cooked`에 에셋 파일이 없으므로, 무엇이 들어갔는지는 `%APPDATA%\Unreal Engine\AutomationTool\Logs\C+UE_5.8\PrePak_Windows_UFSFiles.txt`로 확인한다.
  - 맵은 `MapsToCook`이 비어 있어서 `/Game/Maps` 아래가 모두 쿠킹된다. 그 밖의 에셋은 맵이나 C++(`ConstructorHelpers`)이 참조해야 들어간다. 경로나 PrimaryAssetId로만 찾는 에셋은 따로 쿠킹 규칙이 있어야 한다(아래 `DA_GameData`).
- `.mcp.json`의 `unreal-mcp`(`http://127.0.0.1:8000/mcp`)는 `ModelContextProtocol` 플러그인이 띄우는 서버라서 에디터가 실행 중일 때만 연결된다.
  - 함정(2026-11-06): `set_actor_transform`에 scale만 주면 location / rotation이 0으로 초기화된다(설명과 다르다). 항상 세 값을 모두 준다. 컴포넌트의 Parent Socket(`AttachSocketName`)은 `set_properties`로 읽지도 쓰지도 못하므로 사용자가 Details에서 고른다. 새 맵은 `AssetTools.duplicate`로 템플릿을 복제한 뒤 `save_assets`를 먼저 해야 `load_level`이 열린다.
  - 함정(2026-11-08): 컴포넌트 `BodyInstance`를 `set_properties`로 바꾸면 `CollisionProfileName`만 바뀌고 `CollisionEnabled`는 그대로다. 맵을 저장하고 다시 열어도 프로필대로 맞춰지지 않아서, Details에는 `NoCollision`인데 실제로는 막히는 상태가 된다. 인스턴스 충돌 프리셋은 사용자가 Details에서 바꾼다. 메시 에셋의 `BodySetup.CollisionTraceFlag`와 `StaticMeshTools.remove_collisions`는 잘 된다. 지금 열린 맵에 `load_level`을 다시 부르면 디스크에서 다시 읽지 않는다.
  - 함정(2026-11-08): `set_actor_transform`은 `OnConstruction`을 다시 돌리지 않는다. `AAISpawner::mPatrolPoints`는 `OnConstruction`이 스플라인에서 계산해 월드 좌표로 저장하는 값이라, 스포너를 옮긴 뒤에도 옛 자리를 가리킨다. 아무 프로퍼티나 같은 값으로 `set_properties`(예: `mSpawnDelay`)하면 다시 계산된다. `mSpawnKind`는 건드리지 않는다(`PostEditChangeProperty`가 `mRowName`을 비운다).
- 렌더링은 Substrate(`DefaultEngine.ini` `r.Substrate=True`)라서 머티리얼 Blend Mode 이름이 예전과 다르다. Translucent는 `TranslucentGreyTransmittance`, Modulate는 `ColoredTransmittanceOnly`다(엔진 `EngineTypes.h`에서 같은 값). 가이드에 예전 이름을 적지 않는다(2026-11-04).

## 아키텍처

### 데이터 로딩 (AssetManager → AssetSubsystem)

- `DA_GameData`(`UGameDataAsset`, PrimaryAssetType `GameDataAsset`, `DefaultGame.ini`에 등록)가 모든 DataTable을 `mTables`로 들고 있다.
  - 아무 에셋도 `DA_GameData`를 하드 참조하지 않으므로(런타임에 PrimaryAssetId로 찾는다) Project Settings → Asset Manager의 `GameDataAsset` 타입 **Cook Rule이 `Always Cook`이어야** 패키지에 들어간다. 엔진 `UAssetManager::ModifyCook`은 Always Cook(과 Development 변형)만 쿠킹 목록에 넣고, `Unknown`은 누가 참조할 때만 쿠킹한다. 2026-11-06 패키지에서 `Unknown`이라 DA, DT, DT가 Soft 참조하는 메시 / ABP / 몽타주 / `GA_*` BP가 모두 빠졌고, 데이터 로드 콜백이 오지 않아 캐릭터가 보이지 않고 HP / MP 바가 비었다. DA가 쿠킹되면 나머지는 참조를 따라 함께 들어간다.
- `UAssetSubsystem`(GameInstanceSubsystem)이 시작할 때 비동기 로드하고, 테이블을 **행 구조체 이름**으로 등록한다. 조회는 `FindRow<FXxxTableRow>(RowName)`. 테이블을 추가할 때는 행 구조체를 만들고 `DA_GameData.mTables`에 넣기만 하면 된다(코드 수정 불필요).
- 로드는 비동기라서 BeginPlay 시점에 테이블이 없을 수 있다. 테이블을 읽는 초기화는 반드시 `CallOrRegisterDataLoaded(this, &T::Func)`를 거친다(캐릭터 `InitAttributes`, 스포너 `SpawnCharacter`).
  - 콜백끼리의 순서에 기대지 않는다. 기다리던 콜백은 엔진 멀티캐스트 델리게이트라 **등록 역순**으로 불리고(`MulticastDelegateBase.h` `Broadcast`), 이미 로드된 뒤(레벨 이동 후)에는 등록하는 즉시 불려서 순서가 또 달라진다. 첫 맵에서는 빙의 때 등록한 플레이어 `InitAttributes`(→ `InitAsset`)가 BeginPlay 때 등록한 `UInventoryComponent::InitInventory`보다 늦게 불린다(2026-11-06 로그로 확인).
- 에디터 시점(Details 콤보박스 `GetOptions`)에는 서브시스템이 없으므로 static `UAssetSubsystem::GetRowNamesInEditor(RowStruct)`를 쓴다. AssetManager에 `DA_GameData` 위치를 물어 동기 로드하고, 게임과 같은 규칙(행 구조체)으로 테이블을 고른다. DT 경로를 코드에 적지 않는다.
- 행 구조체 계층: `FCharacterTableRow` → `FPlayerCharacterTableRow`(DT_Player) / `FAICharacterTableRow` → `FMonsterCharacterTableRow`(DT_Monster), `FNPCCharacterTableRow`(DT_NPC). 레벨 스탯은 `FLevelStatTableRow`(DT_LevelStat, 행 이름 `"1"`~`"10"`).
- `DataSource/*.csv`는 각 DT와 같은 내용의 CSV다(첫 열 `---` = 행 이름). 행 구조체 필드를 바꾸면 Claude가 요청이 없어도 CSV를 함께 고친다. DT 에셋은 사용자가 이 CSV로 다시 임포트한다.
  - 형식: UTF-16 LE + BOM, 줄바꿈 LF, 첫 열만 따옴표 없음. 열 순서는 UE Export와 같게 자식 구조체 항목부터 부모 순(`TFieldIterator` 순서)이다.
  - 검사·동기화·값 수정은 `dt-csv-sync` 스킬의 스크립트(`.claude/skills/dt-csv-sync/scripts/dt_csv.py`)로 한다. UTF-16이라 Write/Edit로 직접 고치면 깨진다.

### 캐릭터 계층과 ASC 소유

```
AElmCharacterBase (Abstract)  팀, 장비 메시, DT 외형 적용(InitAsset), 공격 몽타주, Attribute 델리게이트 연결, 피격/사망 처리
├─ APlayerCharacter           ASC 소유자 = AElmPlayerState (Owner=PlayerState, Avatar=캐릭터), DT_Player 행(mRowName)으로 외형/능력치 결정
└─ AAICharacter (Abstract)    ASC 소유자 = 자기 자신(UAIAbilitySystemComponent), DT 행(mRowName)으로 외형/능력치/공격 결정
   ├─ AMonsterCharacter       등급 배율(Epic/Boss) 적용, 처치 시 보상 GE
   └─ ANPCCharacter           IInteractable 구현, 필수 NPC는 쓰러졌다가 부활
```

- 플레이어 생성 흐름: GameMode `DefaultPawnClass`(`APlayerCharacter`, 생성자에서 `mRowName = "Kyle"`) → `PossessedBy`/`OnRep_PlayerState` → `InitAbilitySystem`(ASC 연결, 공격 어빌리티 부여) → (데이터 로드 후) `InitAttributes` → `InitAsset`(서버/클라 모두) → 레벨 스탯(서버만). 캐릭터 전용 클래스는 만들지 않고 DT_Player 행으로 구분한다.
- 스포너는 `AAISpawner` 하나다(몬스터/NPC 전용 스포너 클래스는 만들지 않는다). Details에서 `mSpawnKind`(Monster / NPC)를 고르면 `mRowName` 콤보박스가 그 종류의 DT 행을 보여주고, 만들 클래스와 캡슐 높이도 `mSpawnKind`로 정한다. 종류를 바꾸면 `PostEditChangeProperty`가 `mRowName`을 비우고 `mSpawnDelay`를 종류 기본값(몬스터 0초 / NPC 30초)으로 맞추며, 화살표 색(빨강 / 파랑)은 `OnConstruction`이 정한다.
- AI 캐릭터 생성 흐름: 스포너 `SpawnActorDeferred` → `SetRowName`/`SetSpawner`/`SetPatrolPoints` → `FinishSpawning` → `BeginPlay` → (데이터 로드 후) `InitAttributes` → `FindTableRow`(자식이 공변 반환으로 자기 행 타입을 돌려줌) → `InitAsset`(메시 다음에 ABP) → 공격 어빌리티 부여(DT `AttackAbility`, 비면 `GameplayAbility_Melee`) → 레벨 스탯 → AI 컨트롤러에 감지/공격 거리 전달. 자식 `InitAttributes`는 `Super` 뒤에 전용 값을 덧입힌다.
- 캐릭터가 Destroy되면 `EndPlay` → `AAISpawner::ResetSpawn`(Once면 스포너도 제거, Loop면 `mSpawnDelay` 뒤 재생성).
- 모자/투구/망토 메시는 피벗이 캐릭터 원점이라 소켓에 그냥 붙이면 안 된다. `AttachBindPoseGear`가 본 레퍼런스 포즈의 역행렬로 붙인다.

### GAS

- **태그**는 `ElmGameplayTags.h/.cpp`의 네이티브 태그(`ElmTags::`)로만 정의한다. ini에 추가하지 않는다.
- **GameplayEffect는 에셋이 아니라 C++ 클래스**다(`UGameplayEffect_Damage/Cooldown/AbilityCost/Reward/Recovery`). 수치는 SetByCaller로 넣으며, GE에 SetByCaller Modifier가 여러 개면 모두 채워야 에러 로그가 안 남는다.
- `UElmGameplayAbility`(모든 어빌리티의 부모)
  - `InstancedPerActor`라 인스턴스가 재사용된다. 활성화마다 초기화할 멤버(콤보 인덱스 등)는 `ActivateAbility` 시작에서 리셋한다.
  - `State.Dead`가 있으면 발동이 막힌다.
  - 부모 `ActivateAbility`가 `CommitAbility`(비용 + 쿨다운)까지 한다. 자식은 `Super` 호출 뒤 `IsActive()`로 계속할지 확인한다.
  - 비용/쿨다운은 `mMPCost`/`mHPCost`/`mCooldownTime`/`mCooldownTag` 프로퍼티로 정한다.
  - 몽타주 재생 속도는 `mMontagePlayRate × AttackSpeed` Attribute이고, 엔진이 몽타주의 Rate Scale을 한 번 더 곱한다. 애니메이션 고유의 속도 보정은 어빌리티가 아니라 몽타주 Rate Scale에 둔다(플레이어 3타 몽타주 2.0).
- **공격 어빌리티**: 플레이어 / 몬스터 / NPC가 같은 클래스를 쓴다. `UGameplayAbility_Attack`(Abstract)이 몽타주 재생 → `Event.Attack.*` 대기 → 콤보를 맡고, 자식은 `OnAttackHit`만 구현한다.
  - `UGameplayAbility_Melee`: `ApplyMeleeDamage`(Sweep). DT `AttackAbility`가 비면 모두 이것을 쓴다.
  - `UGameplayAbility_Ranged`: 쏘는 순간 만든 데미지 스펙을 `AElmProjectile`(`Characters/Projectiles/`)에 실어 보낸다. 투사체는 쏜 팀으로 맞힐 대상을 고른다. 조준은 AI면 `SetFocus`된 타겟, 사람이면 카메라 방향(`GetControlRotation`), 그 외에는 정면이다. 투사체 / 소켓은 BP 자식(`Content/Blueprints/GA_*`)에서 정하고 DT `AttackAbility` 열에서 참조한다.
  - 콤보: 몽타주 섹션이 여러 개면 콤보다. `ComboOpen` ~ `ComboClose` 구간에서 다시 발동하면 Retrigger로 다음 섹션이 나가고, 구간 밖 입력은 `CanActivateAbility`가 막는다. 섹션이 하나면 단타다(AI 공격).
  - 실행: 플레이어는 `AttackKey`가 `mAttackAbility` 클래스로 실행한다(ASC가 PlayerState 소유라 캐릭터가 바뀌어도 남으므로 Handle 대신 클래스). AI는 `BTTask_MonsterAttack` → `AAICharacter::PlayAttack`이 Handle로 실행한다.
- **공격 타이밍**: 공격 몽타주에 `Send Gameplay Event` 노티파이(`UAnimNotify_SendGameplayEvent`)로 `Event.Attack.Hit` / `Event.Attack.ComboOpen` / `Event.Attack.ComboClose`를 넣고, 어빌리티가 `WaitGameplayEvent`로 받는다. 이벤트 대기를 몽타주 재생보다 먼저 건다.
  - 노티파이는 **몽타주에만** 둔다. 애니메이션 시퀀스에 두면 그 시퀀스를 쓰는 모든 몽타주에서 함께 울려 한 타에 판정이 두 번 난다(플레이어 3타 몽타주와 Chop 몽타주가 같은 시퀀스를 쓴다).
  - 공격 몽타주는 `Content/Animations`에 두고, 플레이어 / 몬스터 / NPC 모두 DT `AttackMontage` → `InitAsset` → `AElmCharacterBase::GetAttackMontage()`에서 꺼낸다(ABP에 두지 않는다). 피격 몽타주는 ABP Class Defaults의 `mHitMontage`다.
- **데미지 파이프라인**: `MakeDamageSpec`(마법이면 `Damage.Magical` 태그) → `UGameplayEffect_Damage` → `UElmDamageExecution`(공격력은 스펙 생성 시 Snapshot, 방어력은 적용 시점 값, 공식 `공격 × 100 / (100 + 방어)`, 최소 1) → Meta Attribute `IncomingDamage` → `UBaseAttributeSet::PostGameplayEffectExecute`가 HP 차감 후 `mOnDamaged`/`mOnDeath` 발송 → `AElmCharacterBase`가 `State.Dead` 부여 + `CancelAllAbilities` → 자식의 `OnDamagedBy`/`OnKilledBy`. HP를 직접 깎지 않는다.
- AttributeSet 계층은 테이블 행 계층과 1:1로 짝을 이룬다: `UBaseAttributeSet`(공통 전투 스탯) → `UPlayerAttributeSet`(Job, Exp, Gold) / `UAIAttributeSet`(DetectRange) → `UMonsterAttributeSet`(DropExp, DropGold) / `UNPCAttributeSet`(비어 있음).
  - AttributeSet에는 GE가 바꾸는 숫자만 둔다. 숫자가 아니거나 GE로 바꾸지 않는 행 데이터(`DropItems`, `IsEssential`, `DialogueRow` 등)는 캐릭터 멤버로 둔다. `UNPCAttributeSet`은 "NPC 전용 숫자는 여기에 둔다"는 자리라서 비어 있어도 둔다(NPC 전용 값을 `UAIAttributeSet`에 넣으면 몬스터도 갖게 된다).
  - `AAICharacter` 생성자가 `"AttributeSet"` 서브오브젝트를 `UAIAttributeSet`으로 만들고, 자식 생성자가 `ObjectInitializer.SetDefaultSubobjectClass<UMonsterAttributeSet / UNPCAttributeSet>(TEXT("AttributeSet"))`로 바꿔 끼운다. 이름이 `AAICharacter`의 `CreateDefaultSubobject`와 정확히 같아야 하고, 바꿔 끼울 클래스는 원래 클래스의 자식이어야 한다. 이를 위해 `AElmCharacterBase` / `AAICharacter` 생성자가 `FObjectInitializer`를 받아 `ACharacter`까지 넘긴다(기본값 `FObjectInitializer::Get()`이라 인자 없는 `APlayerCharacter` 생성자는 그대로 된다).
  - ASC는 몬스터 / NPC가 `UAIAbilitySystemComponent` 하나를 같이 쓴다. ASC 자식 클래스는 능력 시스템의 동작(입력 연결, 복제 방식 등)이 달라질 때만 만든다. 몬스터와 NPC의 차이(보상, 등급, 부활, 대화)는 캐릭터 클래스에 둔다. 필요해지면 같은 `SetDefaultSubobjectClass`를 `TEXT("AbilitySystemComponent")`로 이어 붙인다.
  - 몬스터 보상은 Killer ASC에 `PlayerAttributeSet`이 있을 때만 준다.

### 팀과 충돌

- 아군/적 구분은 **충돌 채널이 아니라 팀**이 한다. 모든 캐릭터 캡슐은 `Pawn` 프로필이고, 공격 판정은 `ElmChannels::MakeCharacterObjectQuery()`(ECC_Pawn)로 찾은 뒤 `IsHostile`로 거른다.
- 팀 관계는 `AI/TeamTypes.h`의 `SolveTeamAttitude` 관계표 하나가 정한다. 모듈 `StartupModule`에서 `FGenericTeamId::SetAttitudeSolver`로 등록되며, AI 시야와 공격 판정이 모두 이것을 거친다.
- 커스텀 채널은 `Interaction`(ECC_GameTraceChannel1, 기본 Ignore) 하나다. 채널을 추가하면 `ElmCollisionChannels.h`를 `DefaultEngine.ini` 순서와 맞춘다. 채널을 지울 때는 `DefaultEngine.ini`의 `+EditProfiles=`(엔진 기본 프리셋의 커스텀 채널 응답) 줄과 `CollisionChannelRedirects`에 그 이름이 남지 않게 에디터를 끄고 직접 지운다. 남으면 에디터 시작 때 `LogCollisionProfile: ... Custom Channel Name = '...' hasn't been found` 경고가 프리셋마다 난다(2026-11-01 예전 Player/Monster/PlayerAttack/MonsterAttack 잔재를 정리했다).
- KayKit 메시는 임포트 때 만들어진 단순 콜리전이 메시 전체를 덮는 볼록 껍질(convex hull) 하나다. 그래서 문틀, 성문, 언덕의 두 육각형 사이 틈, 나무 묶음 사이처럼 오목한 곳까지 막아 보이지 않는 벽 / 경사면이 된다(캐릭터 이동은 단순 콜리전을 쓴다).
  - **규칙(2026-11-08): 큰 메시는 모양대로, 작은 메시는 통과.** 맵에 놓인 KayKit 메시는 모두 Collision Complexity가 `Use Complex Collision As Simple`이다(지형, 나무, 덤불, 건물, 벽, 바닥, 소품. 지형 계열은 레벨에 없는 것까지 팩 전체). 풀(`ForestNature/Assets/Grass_*`)과 수초(`Natrue/waterlily_*` / `waterplant_*`)만 충돌이 없다: 단순 콜리전을 지우고(Collision → Remove Collision) `Use Simple Collision As Complex`로 둔다. 단순 콜리전만 지우면 Complex 트레이스(AI 시야는 `bTraceComplex`)에 계속 걸리기 때문이다. 새 메시를 맵에 놓을 때도 이 규칙을 따른다.
  - 덤불은 레벨에 놓인 크기가 2 m 이상이라 "큰 메시"로 보았다.
  - 작은 돌: `rock_single_B` / `D`는 Field에서는 10배(높이 1.4 ~ 1.6 m)로, KyleHouse / Tavern(3.2배, 43 ~ 52 cm)과 OrcCave(4배, 54 ~ 65 cm)에서는 자갈 크기로 놓여 있어 에셋 설정으로 나눌 수 없다. 자갈 크기 인스턴스는 레벨에서 Collision Presets를 `NoCollision`으로 바꾼다.
  - Kenney Platformer Kit(`Content/Assets/Kenny/platformer-kit`, 2026-11-09 임포트)도 같은 규칙이다. 임포트 때 Import Collisions를 꺼서 단순 콜리전이 처음부터 없다. 그래서 Remove Collision 없이 Collision Complexity만으로 나눈다: 134개는 `Use Complex Collision As Simple`, 작은 장식 14개(`grass`, `flowers(-tall)`, `plant`, `mushrooms`, `stones`, `coin-*`, `jewel`, `heart`, `star`, `key`, `lock`)는 `Use Simple Collision As Complex`(= 충돌 없음)다. 이 킷의 메시를 Collision Complexity `Default`로 두면 캐릭터가 통과하므로 새로 임포트할 때도 둘 중 하나로 정한다.
    - 임포트 옵션(Interchange FBX): **Asset Type Sub Folders** 켬(텍스처와 머티리얼이 둘 다 `colormap`이라 한 폴더면 엔진이 `...override an asset of a different class` 에러를 내고 하나를 만들지 않는다), **Force All Mesh As Type = Static Mesh**(기본값이면 열기 / 토글 애니메이션이 있는 문 · 레버 · 버튼 · 상자 · 가시 함정 8개가 Skeletal로 들어온다), **Combine Static Meshes = Combine All**(부품이 여러 개인 10개가 갈라지고, `spring.fbx`의 부품 `platform`이 `platform.fbx`와 이름이 겹친다). 캐릭터 `character-*` 5개(0.91 m 스켈레탈)는 넣지 않았다.
    - 크기: 블록 1개 = 1 m, 피벗은 바닥 중앙이다. 킷은 키 0.91 m 캐릭터 기준이고 우리 플레이어는 약 2.2 m(캡슐 반높이 110)라 비율을 맞추려면 ×2로 놓는다. ×2에서 한 칸 오르막(1 m 이하)은 `-low` 블록(0.5 m)이다.
  - 메시의 충돌 설정을 바꾸면 그 메시를 쓰는 맵의 NavMesh가 예전 충돌로 구워진 채 남는다(`DefaultEngine.ini`에 런타임 생성 설정이 없어 정적 NavMesh다). 맵을 열고 Build → Build Paths 후 저장한다.
  - 문 메시(`*_door`)가 없는 `wall_doorway`는 이제 문틀이 열려 있다(Palace / VoidRaindungeon에 `wall_doorway` 5개, 문 2개). 실내 맵 출구 문틀은 그 안에 놓인 문 메시가 막는다.
  - 확인: unreal-mcp `trace_world`는 막힌 구멍도 그냥 통과하므로 쓸 수 없다. 맵을 에디터에 열고 Simulate를 켠 채 `find_actors`(bounds + `ObjectTypeQuery1`) 겹침 검사를 한다(실제 PIE에서 레벨 이동한 뒤에는 `find_actors`가 PIE 월드를 보지 않는다). 겹침 검사는 삼각형 콜리전의 표면만 잡으므로, 상자가 벽 표면을 가로지르게 둔다.

### AI

- `AElmAIController`: AIPerception 시야 + `BT_Monster`/`BB_Monster`(`Content/AI`). 몬스터와 NPC가 같은 BT를 쓰고, 컨트롤러 팀은 조종하는 Pawn을 따른다.
- Blackboard 키 이름은 `MonsterBBKey` 네임스페이스와 BB 에셋이 정확히 같아야 한다.
- `BTService_CheckAttackRange`(0.1초)가 `UpdateTarget`으로 타겟을 고르고, 캡슐 표면 간 거리로 `InAttackRange`를 쓴다. `BTTask_MonsterAttack`은 `AAICharacter::PlayAttack`/`StopAttack`으로 어빌리티를 실행·취소한다.

### 레벨 이동 / 메모(SaveSubsystem)

- GameMode는 `DefaultEngine.ini`의 `GlobalDefaultGameMode`(`AElmGameMode`)라서 맵마다 지정하지 않는다. `ChoosePlayerStart`는 `APlayerStartPIE`("Play From Here") → 태그 없는 첫 PlayerStart → 엔진 기본(무작위) 순으로 고른다.
- `OpenLevel`은 GameInstance와 그 서브시스템 말고는 PlayerState(ASC), 캐릭터(가방)까지 모두 새로 만든다. 그래서 `USaveSubsystem`(GameInstanceSubsystem)이 "메모" `UElmSaveGame`(`Save/ElmSaveGame.h`)를 들고 있고, nullptr이면 새 게임이다.
  - 쓰기는 한 곳이다: `TravelToLevel` → `APlayerCharacter::WriteToSave`. 읽기는 부품마다 자기 초기화 때 한다(`UInventoryComponent::InitInventory`, `APlayerCharacter::InitAttributes`).
  - 이어 갈 상태를 늘리려면 `UElmSaveGame`에 필드를 추가하고 → `WriteToSave`에서 적고 → 그 부품의 초기화(`CallOrRegisterDataLoaded`를 거치는 함수)에서 읽는다.
  - 능력치는 Level / HP / MP / Exp / Gold의 Base 값만 적는다. 나머지는 Level로 `DT_LevelStat`에서 다시 계산된다. `InitLevelStats`(Max 값)보다 먼저 HP / MP를 넣으면 Clamp로 0이 된다.
- 이동: `ALevelPortal`(`Interaction/`, IInteractable)이 `USaveSubsystem::TravelToLevel(Player, Level, StartTag)`를 부른다. URL은 `"맵경로#태그"`이고, 엔진이 그 `PlayerStartTag`를 가진 PlayerStart에 세운다. 태그 = **연결 이름**: 한 문(연결)의 양쪽 끝이 같은 이름을 쓴다. Field ↔ Shop이면 Field 포탈과 Shop 포탈의 `mTargetStartTag`, Field와 Shop의 도착 PlayerStart 태그가 모두 `Shop`이다(`Shop#Shop`, `Field#Shop`). 같은 두 맵 사이에 문이 여러 개여도 구분된다. 예외는 어느 맵에서나 불리는 고정 태그다: `Respawn`(Hospital, 사망 후), `Room`(KyleHouse, 메뉴 "귀환"이 쓴다).
  - 지금 연결 이름: `OrcCave`, `Shop`, `Tavern`, `Hospital`, `HeroHouse`(KyleHouse), `Castle`(Palace). 실내 맵의 포탈은 `Exit_ToTown` 표식 앞 (x, 100, 105), Box Extent 100이다.
  - 페이드 아웃 / 인과 중복 이동 방지(`mIsTraveling` + 입력 끄기)도 `USaveSubsystem`이 맡는다. 도착은 `PostLoadMapWithWorld`로 알고, 로딩 직후 큰 DeltaTime 때문에 검은 화면을 0.5초 유지한 뒤 밝힌다. 카메라 페이드라 HUD(UMG)는 가려지지 않는다.
  - PIE 로그의 `UEDPIE_0_` 접두사 경로는 정상이다.
- 사망: `APlayerCharacter::OnKilledBy` → `AElmGameMode::PlayerKilled` → 2초 뒤 `TravelToLevel(Hospital, "Respawn")`. 메모의 HP가 0이면 죽어서 온 것으로 보고 `InitAttributes`가 HP / MP를 가득 찬 채로 둔다(부활 코드는 따로 없다). 사망 페널티는 없다.
- 디스크 저장: `USaveSubsystem::SaveToDisk` = `WritePlayer` + `SaveGameToSlot(mSaveGame, "ElmSave", 0)`(`Saved/SaveGames/ElmSave.sav`). 메뉴 "저장하기"만 부른다. 이동 중이거나 죽었으면 쓰지 않는다. 불러오기는 아직 없다(메모에 맵 / 위치가 없다).
- 결정 이유와 함정 기록은 끝나서 지운 진행 문서에 있다: `git show d3b7deb:Docs/LevelTravel_Progress.md`.

### Field 맵 (하늘 섬)

- 2026-11-08 "엘름 왕국은 하늘에 뜬 섬" 컨셉으로 바꿨다(StackOBot처럼 섬 사이를 점프로 건넌다). 바꾸기 전 맵은 사용자가 만든 `Field_Backup`이다.
- 땅(2026-11-09): KayKit 육각 타일을 모두 지우고 Kenney Platformer Kit 블록으로 바꿨다(아웃라이너 `SkyIslands/Ground/<섬>`, 라벨 `K_<섬>_<i>_<j>`). 10 m 격자이고 칸 (i, j)의 중심은 ((i + 0.5) × 1000, (j + 0.5) × 1000)이다. +X가 북, +Y가 동이다. 블록 윗면이 섬 높이(본섬 0, 요새 800, 동굴 −600)이고 두께는 10 m다(블록 z = 섬 높이 − 1000).
  - 배율 10(한 칸 = 1 m 블록 하나)이다. 위 Kenney 항목의 "×2"는 캐릭터와 비율을 맞추는 기준인데, 본섬(약 32만 m²)을 ×2로 덮으면 블록이 수만 개라 땅만 ×10으로 놓았다. 징검다리 발판과 언덕 자리의 단은 캐릭터 기준 크기다.
  - 해안 칸: 열린 면이 하나면 `block-grass-overhang-edge`(Yaw 0에서 아래가 깎인 면이 +Y. 열린 면이 +Y / −X / −Y / +X면 Yaw 0 / 90 / 180 / 270), 붙은 두 면이 열렸으면 `block-grass-corner-overhang`(Yaw 0에서 깎인 모서리가 (+X, +Y). (−X, +Y) / (−X, −Y) / (+X, −Y)면 Yaw 90 / 180 / 270). 해안선이 곧게 보이지 않게 해안 블록은 0 ~ 1.2 m 밖으로 밀고 배율 10 ~ 11.6, Yaw ±7°로 흩뜨렸다(윗면 높이는 같다). 가끔 그 아래에 낮은 턱(`KL_*`, `block-grass-overhang-low-large`)이 붙어 있다.
  - 안쪽 칸: `block-grass-overhang-large`(2×2)를 배율 5 / 10 / 20으로 놓아 1 / 2 / 4칸씩 덮는다. 블록 이음선이 땅에 줄로 보이는 것은 킷 모양 그대로다.
  - 해안선은 옛 육각 타일 영역을 흐려서 둥글게 만들고, 건물 / 나무 / 게임용 액터가 놓인 칸은 반드시 땅으로 남겼다. 칸을 더하거나 빼면 이웃 칸의 해안 조각도 위 규칙으로 바꾼다.
- 물(`SkyIslands/Water`): 해자 / 호수 / 강의 육각 물 타일을 지우고 평평한 판 하나씩(엔진 `Cube` + `MI_Water`. `Kenny/platformer-kit/Materials`, 부모 `/Engine/BasicShapes/BasicShapeMaterial`)으로 바꿨다. 윗면은 예전 물 높이 그대로(해자 −2 m, 호수 −0.5 m, 강 −0.8 m)이고, 판이 물 칸보다 넓어서 둑 블록 아래로 들어가 있다. 판이 섬 가장자리 밖으로 나오면 보이므로 강은 해안 한 칸 앞에서 끝난다.
- 길: 옛 길 타일 자리에 Kenney `stones`(충돌 없음)를 깔았다(`SkyIslands/PathStones`).
- 언덕: KayKit 언덕(`hills_*`, `hill_single_*`) 75개를 Kenney 단으로 바꿨다(`SkyIslands/Plateaus`). 높이 1.2 m(바로 점프)이거나, 2.4 m에 1.2 m 발판(`PlateauStep_*`)이 붙어 있다. `*_trees`였던 자리는 위에 KayKit 나무를 놓았다.
- 남은 KayKit 지형: 야영지 / 요새 안뜰의 `building_dirt`(흙 육각 바닥 6개)와 동굴 섬의 산(`mountain_*_grass_trees`, KayKit 풀색)은 그대로다.
- 섬:
  - 본섬(z 0): 마을, 호수(북), 서쪽 폐허, 야영지. 해안은 둥글게 들쭉날쭉하다(반지름 약 300 ~ 400 m). 원래 가장자리를 막던 산 고리(24번째 고리)와 숲 고리(23번째)는 지웠다.
  - 그림자 요새 섬(남): 원래 (20, 0) 둘레 5칸(앞 야영지, 폐허 포함)을 X −50 m, Z +8 m 옮겼다.
  - 동쪽 동굴 섬: 원래 (9, −18) 둘레 4칸을 Y +35 m, Z −6 m 옮겼다. 포탈 `OrcCave`, 도착 PlayerStart, 스포너 4개(`AISpawner_1 / 2 / 3 / 5`), 태그 없는 `PlayerStart_0`도 함께 옮겼다. 그래서 Play From Here 없이 Field를 PIE로 시작하면 동굴 섬에서 시작한다.
  - 장식용 작은 섬 9개: 지운 가장자리 칸 몇 개를 바깥 / 위아래로 옮긴 것이다.
- 섬 사이 길(아웃라이너 `SkyIslands/Paths`): 본섬 → 요새 `Step_Fort_00 ~ 07`(발판 8개, 한 칸 +0.89 m, 가장자리 간격 약 2.2 m, 출발점에 빨간 깃발), 본섬 → 동굴 `Step_Orc_00 ~ 06`(7개, 한 칸 약 −0.86 m, 간격 약 2.6 m, 초록 깃발). 가운데 발판 하나는 넓은 쉼터(나무 + 돌)다. 발판은 `block-grass-overhang-large`(배율 1.5 ~ 3.8, 두께 1.5 ~ 2.5 m)이고 아래에 뒤집은 산(`*_Rock`)이 붙어 있다. `Step_Orc_06`(윗면 −5.57 m)은 2026-11-09 해안선을 다시 그리면서 동굴 섬 해안이 마지막 발판에서 4.8 m 멀어져 더한 것이다.
  - 점프 기준(DT_Movement `Player`): 끝까지 누르면 높이 약 1.6 m, 평지 수평 거리 약 4.5 m, 1 m 높은 곳에 내릴 때 약 3.7 m. 발판을 더할 때 오르막은 한 칸 1 m 이하, 가장자리 간격은 2.6 m 이하로 둔다.
- 장식: `SkyIslands/Underside`(땅 블록 아래 뒤집은 `mountain_A / B / C`, Pitch 180, 라벨 `U_<섬>_<i>_<j>`. 해안 칸마다 작은 것, 그 안쪽 줄은 한 칸 걸러 큰 것, 더 안쪽은 50 m 간격으로 깊은 것이 있고 작은 섬은 가운데에 하나 더 있다. 해안을 바꾸면 그 칸의 것도 고친다), `SkyIslands/Floaters`(길 옆 떠다니는 Kenney 발판), `SkyIslands/Clouds`(섬 아래 50 ~ 90 m의 구름 바다와 먼 구름). 구름 메시(`cloud_big` / `cloud_small`)는 풀과 같은 규칙으로 충돌을 없앴다.
- 떨어지면: WorldSettings KillZ가 엔진 기본값(−1048575)이라 섬 밖으로 떨어지면 끝없이 떨어진다. KillZ만 올리면 엔진 `AActor::FellOutOfWorld`가 캐릭터를 Destroy해서 조종할 Pawn이 없어진다. 낙하 처리(`APlayerCharacter::FellOutOfWorld` 재정의)를 만든 뒤에 KillZ를 정한다(2026-11-08 미구현).
- AI는 NavMesh가 끊긴 섬 사이를 건너지 못한다(의도). 섬 배치를 바꾸면 Build → Build Paths 후 저장한다.

### SkyIslands 맵 (Kenney 하늘섬, 2026-11-09)

- `/Game/Maps/ElmKingdom/SkyIslands`: Field와 별개인 새 맵이다. 엔진 `Template_Default`를 복제해서 조명은 Field와 같고 템플릿의 `Floor`만 지웠다. 게임용 액터는 남문 안쪽 큰길의 `PlayerStart`(-6600, 0, 120, 성을 본다) 하나뿐이다(NavMesh / 스포너 / 포탈 / 보스 없음, KillZ 기본값이라 떨어지면 끝없이 떨어진다).
- **아트 규칙(2026-11-09 사용자 지시): 건축물은 KayKit만, 지형지물(땅 / 나무 / 바위 / 풀 / 길 돌)은 Kenney만 쓴다.** 건물 옆 소품(통 / 상자 / 깃발 / 무기대 / 과녁 / 목재 / 수레)은 건물과 맞춰 KayKit(×6, 깃발 ×16)이고, 남쪽 섬의 짐 상자는 Kenney다.
- **지형은 블록만 깔끔하게 깐다(2026-11-09 사용자 지시).** 섬 밑면에 흙 블록을 쌓거나 흙 기둥을 매다는 장식을 넣지 않는다. v3에서 섬 밑면 / 징검다리 밑에 `SM_DirtBlock` 98개를 붙였다가 사용자가 지웠다. 그때 만든 `SM_DirtBlock` / `MI_KenneyDirt`(`platformer-kit/StaticMeshes`, `Materials`)는 지금 아무도 참조하지 않는다.
- **섬 하나 = 크게 늘린 블록 하나**(2026-11-09 사용자 지시). 처음에는 4 m 블록을 격자로 이어 붙였는데 이음선이 줄로 보여 바꿨다. 언덕의 단도 단마다 블록 하나(두께 2 m, 바닥이 아래 단 속에 묻힌다)다. 땅을 고칠 때 블록을 이어 붙이지 않는다.
  - 땅 블록은 `block-grass-overhang-large`(배율 1에서 한 변 208.2 cm, 피벗은 바닥 가운데)다. 소품 / 다리 / 징검다리는 캐릭터 기준 ×2 배율이다. +X 북, +Y 동.
  - 크게 늘리면 가장자리 풀 테두리가 경사가 된다: 수평 폭 = 4.1 cm × xy 배율, 높이 = 12.5 cm × z 배율(trace로 잼). 가운데 섬(xy 105.7, z 18)은 가장자리 4.33 m가 2.25 m 내려가는 경사(27°, 걸어 다닐 수 있다)라, 다리 판자는 섬 위로 4.83 m 걸친다. 징검다리 첫 돌은 돌 바닥(윗면 − 1.2 m)이 경사면보다 20 cm 위에 오는 가장 가까운 자리에 둔다(생성기 `first_gap`. 동쪽 +0.5 m 돌은 경사 위에 걸쳐 평지 끝에서 1.73 m, 서쪽 −0.8 m 돌은 4.24 m). 동쪽 섬 경사(2.36 m 폭, 1.75 m 높이, 48°)는 걸을 수 없으므로 마지막 돌은 풀 테두리 끝에서 0.3 m만 띄웠다(평지까지 2.66 m, +0.5 m). 경사 위에 길 돌 같은 평평한 소품을 두면 떠 보인다. 풀 늘어짐 모양도 같이 커진다(가운데 섬은 한 물결이 수 m).
- KayKit 건물은 Field와 같은 ×10(성 ×13)이다. **정면 = 로컬 +Y**(Field 문 포탈 위치로 확인)라서 바라볼 방향이 θ면 Yaw = θ − 90이다. 성문에는 문짝을 달지 않았다(열린 성문).
- **성벽은 Field와 같은 육각형이다**(2026-11-09 사용자 지시. 네모 → 원형("어색하다") → 육각형). 생성기 `hex_wall`이 Field `ElmField/Walls`에서 읽은 규칙을 그대로 쓴다(생성기가 Field 마을 성벽 44개를 다시 만들어 맞는지 스스로 검사한다, `field_selftest`):
  - 모든 조각 ×10, 늘이거나 줄이지 않는다. 꼭짓점은 중심에서 n × 20 m(한 변 n칸), 각도 θ0 + 60k.
  - 꼭짓점: `wall_corner_A_inside`(Yaw = 꼭짓점 각 − 60). 성문 꼭짓점은 `wall_corner_A_gate`(같은 Yaw) + 양옆 변 쪽 10 m에 탑(`tower_A` + `_top`, 같은 Yaw).
  - 변: 두 꼭짓점 사이를 n등분한 자리마다 `wall_straight`(Yaw = 변 바깥 법선 + 90). 변 가운데 성문은 같은 자리 / Yaw의 `wall_straight_gate`(n이 짝수여야 변 가운데에 칸이 있다).
  - 이 방향이면 `wall_straight`의 갈색 나무 부분(로컬 −Y)이 바깥을 본다(Field와 같다).
  - 원형(v5)은 직선 벽을 여러 각도로 꺾어 이었고, 네모(v4)는 모퉁이에서 벽을 겹쳤다. 둘 다 KayKit 모퉁이 조각을 쓰지 않아 어색해 보였다.
- 섬(크기 / 윗면 높이):
  - 가운데(성과 마을): 220 m 정사각형, 0 m, 두께 18 m. **성벽이 성과 마을을 함께 둘러싼다**(2026-11-09 사용자 지시). 육각 성벽은 Field와 같은 방향(꼭짓점이 북 / 남)으로 한 변 4칸(꼭짓점 80 m, 변심 69.3 m)이다. 북 / 남 꼭짓점이 탑 낀 모퉁이 성문(다리 쪽), 동 / 서 변 가운데가 성문(징검다리 쪽)이고, 성문 밖에서 섬 끝까지 길 돌이 깔려 있다.
    - 성벽 안: 한가운데에 파란 성(`building_castle_blue` ×15, 정면 남쪽), 성 둘레 길(반지름 32 m), 성 앞 광장(우물, 깃발), 성문 넷으로 가는 큰길. 마을 24채: 큰길가 16채(주점 / 시장 / 제재소 / 병영 / 활터 / 교회 / 집)는 길을 보고, 큰길 사이 안쪽 8채(45° 방향 반지름 52 m에 대장간 포함 4채, 육각 모서리 방향 반지름 56 m에 4채. 더 밖은 모퉁이 성벽 조각이 차지한다)는 성을 본다. 큰길 사이 성 가까이는 정원(나무 / 풀)이다. 마을은 성벽 안쪽 면에서 1.8 m 이상 안에 있다(생성기 `outside_walls`가 육각형 기준으로 검사).
    - 성벽 밖 네 모서리: 북동 계단 언덕(단 4개, +1 ~ +4 m, 꼭대기 큰 나무), 북서 대지(+1 / +2, 위에 풍차), 남서 / 남동 숲. 언덕 / 대지는 성벽에서 10 m 이상 밖이다(`tiers_vs_walls` 검사).
  - 북(29 × 31 m, 0, 숲, +1 둔덕): 다리 20 m. 남(27 × 30 m, 0, 짐 상자): 다리 16 m. 다리는 `platform` ×2 판자(가운데가 22 cm 처짐) + 양쪽 `fence-rope` 난간 + 양 끝 `flag`.
  - 동 = **최종 보스 성**(110 × 120 m, **+10 m**, 두께 14 m): 육각 성벽(한 변 2칸 = 꼭짓점 40 m, 꼭짓점이 ±Y, 중심이 섬 가운데보다 2 m 뒤(+Y))이 빨간 성(`building_castle_red` ×13, 정면 −Y, 성벽 중심보다 4 m 뒤)을 둘러싼다. 성문은 −Y 꼭짓점(징검다리 쪽)의 모퉁이 성문 + 양옆 `tower_A_red`다. 성문과 성 사이(약 25 m 깊이)는 비워 둔 전투 마당이고, 성벽 밖은 침엽수 / 바위다. 징검다리 11개(+0.5 m에서 0.9 m씩 올라 9.5 m, 섬까지 +0.5 m, 간격 1.4 ~ 1.5 m, 6번째는 깃발 꽂힌 넓은 쉼터 돌).
  - 서 = **중간 보스 던전**(52 × 52 m, **−8 m**, 두께 12 m): 먼 쪽(−Y)에 절벽 단 2개(+3 / +6 m, 두께 3.5 m)를 두고 그 앞에 광산(`building_mine_blue` ×12, 입구가 +Y = 징검다리 쪽, 뒤쪽이 절벽에 묻힘), 입구 앞이 전투 마당이다. 왼쪽 앞에 폐허(`building_destroyed`), 큰 바위 / 버섯. 징검다리 9개(−0.8 m씩, 간격 1.5 m, 5번째 넓은 쉼터 돌).
  - Field의 점프 기준(오르막 1 m 이하, 간격 2.6 m 이하) 안이다(생성기 보고서의 `stones_E` / `stones_W`). 작은 섬은 두께 10 m라 가장자리 경사가 약 0.55 m 폭, 1.25 m 높이다.
- 아웃라이너 `SkyIslands/<Central|North|South|East|West>`(`<섬>_Ground` + `Tiers` / `Props`), 가운데 섬은 `Castle` / `Walls` / `Roads` / `Village`(+ `Props`), 동쪽 `Fortress`, 서쪽 `Dungeon`. `SkyIslands/Bridges/<North|South>`, `SkyIslands/Stones/<East|West>`.
- 처음 맵에 놓은 KayKit 메시 16개(풍차 3부품, 활터, 제재소 3부품, `tower_B_blue`(+top), 투석기 탑 3부품, `crate_B_big`, `resource_stone`, `pallet`, `flag_red`)는 충돌 규칙대로 `Use Complex Collision As Simple`로 바꿨다. `flag_red`는 Field에서도 쓰므로 Field NavMesh를 다시 구워야 한다(아직 안 함).
- 배치는 생성기 `Saved/SkyIslandsGen/gen_skyislands.py`(v6. 이전 판 `_v2.py` ~ `_v5.py`. gitignore 대상이라 만든 PC에만 있다)가 만든 JSON을 unreal-mcp로 읽어 놓았다. 판을 바꿀 때는 이전 판 결과와 비교해서 바뀐 액터만 지우고 놓는다(v5 → v6은 1,061개 중 376개를 그대로 두고 685개 삭제, 620개 추가). 생성기가 건물 / 성벽 / 길 / 소품 바닥 다각형의 겹침, 성 둘레 길 침범, 성벽 밖으로 나감, 언덕과 성벽 거리, 섬 가장자리, 단 받침, 첫 / 마지막 징검다리 거리를 검사해 보고한다. 난수는 구역(섬 / 징검다리 / 다리 / 섬별 나무·풀)마다 따로라서 한 구역을 고쳐도 다른 구역 배치는 그대로다. 섬 크기나 위치를 바꾸면 다리 길이 / 징검다리 간격 / 소품이 섬 크기에서 계산되므로 생성기를 고쳐 다시 놓는 편이 빠르다(액터 996개. `add_to_scene_from_asset` + `set_actor_folder`가 액터당 약 0.5초, `remove_from_scene`은 약 0.3초라서 한 번의 스크립트 호출에 110개 묶음 2 ~ 3개씩 놓았다).
  - 사용자가 맵을 손본 뒤 다시 놓을 때는 먼저 맵의 액터 라벨 / 트랜스폼을 생성기 결과와 비교해서(`AssetTools.write_file`로 `level_state.json`에 받아 비교) 사용자 수정이 지워지지 않는지 확인한다.

### 메인 메뉴 레벨

- `/Game/Maps/MainMenu`(2026-11-06): 엔진 Basic 템플릿(`/Engine/Maps/Templates/Template_Default`)을 복제한 배경 전용 맵이다. 조명은 Field와 같은 템플릿 값이고 태양(`DirectionalLight`) 방향만 캐릭터 얼굴이 화면 왼쪽 위에서 비치게 돌렸다. 배경은 Field의 성 안쪽(성벽 포함) 복사본이다. Field 원점 중심 반지름 6칸(성벽이 놓인 줄)까지의 StaticMeshActor 287개를 같은 메시 / 트랜스폼 / 라벨 / 아웃라이너 폴더(`ElmField/*`, `ElmKingdom/*`)로 다시 놓았다(전부 머티리얼 덮어쓰기 없는 Static 단일 컴포넌트라 메시와 트랜스폼만 옮기면 같다). 문 표식 / 포탈 / PlayerStart / 스포너 / NavMesh 같은 게임용 액터는 뺐다. 그래서 Field의 성 안쪽을 고치면 MainMenu는 따로 고쳐야 한다. 성벽 바깥은 비어 있어서 높은 곳에서 보면 섬처럼 끝난다.
- 구도: `Kyle`은 남문 큰길 (-9100, -300), 카메라는 (-10000, -175, 210)에서 +X(성 쪽)를 본다. 카메라를 이보다 남문 쪽으로 물리면 카메라 뒤 남문 탑 그림자가 화면 아래에 걸린다.
- 액터를 수백 개 한꺼번에 만든 직후의 뷰포트 캡처는 하얗게 과노출되어 보였고, 맵을 다시 열자 Field와 같은 노출로 돌아왔다(2026-11-06). 조명 값이 아니라 갱신 지연이었다.
- 표시용 주인공은 게임 캐릭터가 아니라 `Kyle`(SkeletalMeshActor: Knight 메시 + `Rig_Medium_GeneralIdle_A` 단일 노드 재생)이다. 칼은 그 안의 `Sword` 컴포넌트를 Parent Socket `handslot_r`에 붙인다(게임과 같은 소켓).
- 카메라는 `MenuCamera`(CameraActor, FOV 60, Constrain Aspect Ratio 끔)다. **Auto Activate for Player = Player 0**이라 코드 없이 뷰 타깃이 된다(엔진 `APlayerController::AutoManageActiveCameraTarget`이 Pawn보다 먼저 이 카메라를 고른다).
- 아직 GameMode Override가 없어서 PIE에서는 게임용 `AElmGameMode`가 돌아 HP / MP HUD가 뜨고, 게임 캐릭터는 카메라 뒤 `PlayerStart`(-11200, 0)에 생긴다.

### 입력 / 상호작용 / UI

- 입력 설정은 `UElmInputConfig` 자식 하나가 IMC 하나와 그 IA들을 생성자에서 경로로 찾아 `FindInputAction("Name")`으로 제공한다: `UDefaultInput`(`Content/Input/Default/`, `IMC_Default`), `UDialogueInput`(`Content/Input/Dialogue/`, `IMC_Dialogue`, 대화 중에만 `IMC_Default` 대신 켠다). 입력을 추가하려면 해당 폴더에 IA를 만들고 → 그 입력 설정 생성자에 등록하고 → 캐릭터 동작은 `APlayerCharacter::SetupPlayerInputComponent`, UI/상호작용은 `AElmPlayerController::SetupInputComponent`에서 바인딩한다. 핸들러가 컨트롤러 컴포넌트의 함수여도 바인딩은 컨트롤러에서 한다(`mInteraction.Get()`처럼 원시 포인터로 넘긴다).
- 상호작용: `IInteractable`(C++ 전용 인터페이스)을 구현하고 해당 컴포넌트가 `Interaction` 채널에 Overlap으로 응답하면 된다. `UInteractionComponent`(`Player/`, `AElmPlayerController`에 붙음)가 0.1초마다 구 Overlap으로 가장 가까운 대상을 찾아 HUD에 `[F] ...` 문구를 띄운다. 키 글자는 `UElmInputConfig::FindMappedKeyText`가 지금 켜진 IMC에서 물어 온다. 다른 기능이 상호작용을 막을 때는 `SetBlocked(true / false)`를 부른다(상호작용 컴포넌트는 누가 막는지 모른다).
- 플레이어 컨트롤러에 기능을 더할 때는 `Player/`의 ActorComponent로 만들어 컨트롤러에 붙이고, 컨트롤러에는 생성과 키 바인딩만 둔다(2026-10-30 상호작용 / 대화를 이렇게 뺐다). 컴포넌트 `BeginPlay`는 컨트롤러 `BeginPlay`의 `Super` 안에서 불려 아직 HUD가 없다.
- HUD: `AElmPlayerController`가 `WBP_PlayerHUD`(`UPlayerHUDWidget`)를 만든다. C++ 위젯 변수는 `meta = (BindWidget)`이며 WBP의 위젯 이름과 같아야 한다. 상태바 같은 위젯은 `GetOwningPlayerState()`의 ASC Attribute 델리게이트를 구독해서 갱신한다. 창을 열고 닫을 때 입력 모드/커서는 `RefreshInputMode`가 맞춘다.
- 메뉴(B, `IA_Menu`): `AElmPlayerController::SetMenuOpen`이 `WBP_PlayerHUD`의 자식 `mMenuWidget`(`UMenuWidget` / `WBP_Menu`)을 켜고 `SetPause`로 게임을 멈춘다. 버튼은 일을 맡은 곳에 부탁만 한다: 귀환 → `AElmGameMode::Recall`(KyleHouse `Room`), 저장 → `USaveSubsystem::SaveToDisk`, 종료 → `QuitGame`(확인 상자). 귀환 / 저장은 살아 있고 전투 중이 아닐 때만 켠다(`AElmGameMode::IsInCombat` = 살아 있는 AI 중 Blackboard `Target`이 이 플레이어인 것이 있음).
  - 일시정지 중에는 **Trigger when Paused**를 켠 IA만 발동한다. `IA_Menu`만 켜 두어서 메뉴가 열린 동안 이동 / 공격 / I / F가 막힌다. 입력 모드는 `FInputModeGameAndUI`다(UIOnly면 B가 컨트롤러에 오지 않아 메뉴를 닫을 수 없다).
  - 일시정지 중에는 월드 타이머가 돌지 않는다. 타이머를 쓰는 일(`TravelToLevel`의 페이드)은 메뉴를 닫은 뒤에 부른다(`UMenuWidget::OnRecallClicked`).
  - 메뉴 버튼은 WBP에서 **Is Focusable**을 끈다. 켜 두면 클릭한 버튼이 포커스를 가져가 Space / Enter가 그 버튼을 다시 누른다.
- 창(인벤토리 I, 퀘스트 P, 캐릭터 C)을 더하는 길: IA + `IMC_Default` 매핑 → `UDefaultInput` 등록 → 컨트롤러 바인딩 → `SetXxxOpen` → HUD `SetXxxVisible` / `IsXxxVisible` → `IsAnyWindowOpen`에 추가(→ `RefreshInputMode`가 커서와 GameAndUI를 맞춘다). IA는 빌드 전에 만들어 저장한다(입력 설정 CDO가 에디터 시작 때 경로로 찾는다).

### 대화 / 퀘스트

- 대화: `UDialogueComponent`(`Player/`, 컨트롤러에 붙음, `GetDialogue()`). NPC의 `Interact`가 시작 행을 골라 `StartDialogue(NPC, Row)`를 부른다. 대사는 `DT_Dialogue`(`FDialogueTableRow`: `Speaker` / `Text` / `NextRow`) 한 줄이 한 행이고 `NextRow`가 None이면 끝이다. 선택지는 없다.
  - 대화 중에는 `IMC_Default`를 빼고 `IMC_Dialogue`(F / Space / 마우스 왼쪽 → `IA_DialogueNext`)를 넣어 다른 입력을 한 번에 막는다. IMC를 바꾸는 순간 눌려 있던 키를 무시하는 기본 옵션은 Value Type이 Boolean인 IA에만 걸리므로 `IA_DialogueNext`는 Digital(bool)로 둔다. 커서는 띄우지 않는다.
  - 시작할 때 인벤토리 / 퀘스트 / 캐릭터 창을 닫고 `UInteractionComponent::SetBlocked(true)`를 부른다. NPC는 `BeginTalk`(BT `PauseLogic`, 플레이어 바라보기) / `EndTalk`(`ClearFocus` → `ResumeLogic`)로 멈췄다가 돌아간다. `EndTalk`는 NPC가 쓰러져 있어도 반드시 `ResumeLogic`을 부른다(사망 때의 `StopLogic`은 일시정지를 풀지 않아서, 안 풀면 부활 뒤에도 BT가 멈춰 있다).
  - 끊김: 대화 중에만 도는 0.1초 타이머(`CheckInterrupt`: 플레이어 사망, NPC `CanInteract` false = 쓰러짐 / 전투)와 `APlayerCharacter::OnDamagedBy`가 `EndDialogue`를 부른다.
- 퀘스트: `UQuestComponent`(`Player/`, 컨트롤러에 붙음, `GetQuest()`)가 받은 퀘스트 목록(`TArray<FQuestProgress>`: `QuestRow` / `State` / `KillCount`)을 들고, 받기 / 처치 세기 / 완료와 보상을 맡는다. 데이터는 `DT_Quest`(`FQuestTableRow`: 제목, 설명, `TargetMonster`(DT_Monster 행) × `TargetCount`, `RewardExp` / `RewardGold`, `ProgressDialogue` / `CompleteDialogue`). 목표는 처치형 하나뿐이다.
  - 받기: DT_NPC `QuestRow`가 있는 NPC와의 대화를 마지막 줄까지 넘기면 `UDialogueComponent::AdvanceDialogue`가 `AcceptQuest(NPC의 QuestRow)`를 부른다. 전투·피격으로 끊기면 받지 않는다. 수락 / 거절 UI는 없다(2026-11-02 대화창 버튼으로 만들다가 입력 모드·커서 처리까지 늘어나 사용자가 뺐다).
  - 다시 말 걸기: `ANPCCharacter::ChooseDialogueRow`가 진행 상태로 시작 행을 고른다(받기 전 = DT_NPC `DialogueRow`, 진행 중 / 완료 = DT_Quest `ProgressDialogue` / `CompleteDialogue`, 비어 있으면 `DialogueRow`).
  - 처치: `AMonsterCharacter::OnKilledBy` → `NotifyQuestKill`(잡은 캐릭터의 컨트롤러가 플레이어일 때만, 보상과 같은 규칙) → `NotifyMonsterKilled(mRowName)`. 목표 수를 채우는 순간 완료되고 `UGameplayEffect_Reward`(SetByCaller Exp / Gold)를 플레이어 ASC에 적용한다. NPC에게 보고하러 가지 않는다.
  - 레벨 이동: 메모 규칙 그대로다. `APlayerCharacter::WriteToSave`가 `UElmSaveGame::Quests`에 적고, `UQuestComponent::InitQuests`(`CallOrRegisterDataLoaded`)가 읽는다.
  - UI: HUD 오른쪽 `WBP_QuestTracker`(`UQuestTrackerWidget`, 진행 중만)와 P 키 창 `WBP_Quest`(`UQuestWidget`, 진행 중 + 완료)가 `AddQuestChangeCallback`으로 컴포넌트를 구독해서 스스로 다시 그린다. 레벨 이동 직후에는 HUD가 생기기 전에 목록이 되살아나므로 위젯이 `NativeConstruct`에서 한 번 직접 읽는다. 목표 글자("해골전사 처치 0 / 1")는 `UQuestComponent::MakeObjectiveText`가 만든다.

### 인벤토리 / 장비

- 아이템은 가방 / 장비칸의 아이콘(`UItemObject` + `DT_Item` 행)으로만 있다. 레벨에 놓이는 아이템 액터와 줍기는 없고, `FItemTableRow`에 메시도 없다(2026-11-05 결정). 몬스터 드랍은 처치할 때 가방에 바로 넣는다.
- `UInventoryComponent`(`Player/`, 플레이어 캐릭터의 `mInventory`)가 가방 `mItemList`(50칸)와 장비칸 `mEquipList`(칸 번호 = `EEquipSlot`: 무기 / 갑옷 / 목걸이)를 함께 든다. 장비한 아이템은 가방에서 빠진다. 가방 칸 우클릭 / 더블클릭(`UseItem`)이 착용이고 끼고 있던 장비와 맞바꾼다.
  - 아이콘 클릭은 `UIconWidget::UseSlotItem`이 칸 종류(`ESlotType`)별로 나눈다. 더블클릭의 두 번째 누름은 `NativeOnMouseButtonDown`으로 오지 않고 `NativeOnMouseButtonDoubleClick`으로만 온다(엔진 `FSlateApplication::ProcessMouseButtonDoubleClickEvent`). 그래서 빠른 우클릭 두 번도 더블클릭 함수가 받아야 두 번째가 사라지지 않는다. `UnequipItem`은 가방이 가득 차 있으면 실패한다.
- 장비 능력치는 칸마다 Infinite GE(`UGameplayEffect_Equipment`, SetByCaller 4개) 하나를 걸고 `mEquipEffectList`의 Handle로 지운다. Base 값을 고치지 않으므로 레벨업(`InitLevelStats`)과 메모(Base 값만 적는다)에 섞이지 않는다.
- 장비는 겉모습을 바꾸지 않는다. 플레이어 무기 메시는 DT_Player `MainhandMesh` 하나이고, 무기칸은 그 메시를 보이거나 숨기기만 한다(`APlayerCharacter::SetWeaponVisibility`). 그래서 `InitAsset`과 `InitInventory` 중 어느 쪽이 먼저 불려도 결과가 같다.
- 레벨 이동은 메모 규칙 그대로다(`UElmSaveGame::Inventory` / `Equipment`). GE는 이전 ASC와 함께 사라지므로 `InitInventory`가 장비칸마다 다시 건다.
- 캐릭터 창(C, `UCharacterInfoWidget` / `WBP_CharacterInfo`)은 장비 / 능력치 두 탭이다(`mTabSwitcher` 0 / 1, 지금 탭의 버튼을 꺼서 표시한다).
  - 장비칸 3개는 `ESlotType::Equipment` 슬롯이고, 슬롯 번호 = `EEquipSlot`이다. 그래서 아이콘이 번호를 `GetEquipItem` / `UnequipItem`에 그대로 넘긴다(우클릭 / 더블클릭 = 해제).
  - 끌어다 놓기는 가방 칸끼리만 된다(`USlotWidget::NativeOnDrop`). 장비칸 번호를 `SwapItem`에 넘기면 엉뚱한 가방 칸끼리 바뀌기 때문이다.
  - 장비 몫은 `현재 값 − Base 값`으로 계산해 `30 (+10)`처럼 보여 준다. 위젯이 인벤토리의 장비 변경과 PlayerState ASC의 Attribute 변경을 구독해서 스스로 다시 그린다.
  - 아이템 정보창은 칸 종류로 자리를 정한다(`UPlayerHUDWidget::ShowItemInfoWidget(IsShow, From)`). 장비칸이면 캐릭터 창 오른쪽, 가방이면 인벤토리 왼쪽에 붙는다.

## 코드 컨벤션

- 멤버 변수는 `m` 접두사(`mASC`, `mTeam`)를 쓰고, bool은 `b` 대신 `Is`를 쓴다(`mIsEssential`, `IsMagical`).
- 프로젝트 헤더는 상대 경로로 include하고(`"../GameInfo.h"`), 대부분의 헤더가 `GameInfo.h`를 먼저 include한다.
- 로그 카테고리: `LogTestDebug`(경고/에러/디버그), `LogTestOrder`(초기화 호출 순서 추적).
- 주석은 한글로, "왜 이렇게 하는지"를 적는다. 한 줄짜리 `if` 본문은 중괄호를 생략하는 스타일이다.
- 소스 파일은 UTF-8(BOM)로 저장한다. 2026-11-02 새로 만든 `QuestTrackerWidget.cpp`가 CP949(ANSI)로 저장되어 git diff와 다른 도구에서 한글이 깨졌다. UBT는 `/utf-8`로 컴파일하므로(엔진 `VCToolChain.cs`) CP949 파일의 한글은 컴파일러에게도 깨진 글자다. 주석뿐이면 빌드는 되지만 `NSLOCTEXT` / `TEXT` 문자열이면 게임 화면 글자가 깨진다. 2026-11-06에 `CelebrationWidget.cpp`(CP949), `MenuWidget.cpp`(BOM 없는 UTF-8)와 ASCII 파일 7개를 UTF-8(BOM)으로 바꿨다. VS에서는 파일 → 다른 이름으로 저장 → 저장 버튼 옆 ▼ → 인코딩하여 저장 → "유니코드(서명 있는 UTF-8) - 코드 페이지 65001"로 바꾼다. 한글이 든 `NSLOCTEXT` / `TEXT` 문자열이 있는 파일은 특히 지킨다.
- 생성자에서 에셋은 `ConstructorHelpers`로 전체 경로(`/Script/...'...'`)를 써서 찾는다. 클래스(ABP/WBP)는 경로 끝에 `_C`를 붙인다. 테이블로 정할 수 있는 외형/몽타주/어빌리티는 생성자 대신 DT의 Soft 참조를 쓴다.
