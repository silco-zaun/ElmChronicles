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
- 자동화 테스트나 린트 설정은 없다. 검증은 PIE로 한다. 시작 맵은 `/Game/Maps/Test`다(`DefaultEngine.ini`).
- `.mcp.json`의 `unreal-mcp`(`http://127.0.0.1:8000/mcp`)는 `ModelContextProtocol` 플러그인이 띄우는 서버라서 에디터가 실행 중일 때만 연결된다.
- 렌더링은 Substrate(`DefaultEngine.ini` `r.Substrate=True`)라서 머티리얼 Blend Mode 이름이 예전과 다르다. Translucent는 `TranslucentGreyTransmittance`, Modulate는 `ColoredTransmittanceOnly`다(엔진 `EngineTypes.h`에서 같은 값). 가이드에 예전 이름을 적지 않는다(2026-11-04).

## 아키텍처

### 데이터 로딩 (AssetManager → AssetSubsystem)

- `DA_GameData`(`UGameDataAsset`, PrimaryAssetType `GameDataAsset`, `DefaultGame.ini`에 등록)가 모든 DataTable을 `mTables`로 들고 있다.
- `UAssetSubsystem`(GameInstanceSubsystem)이 시작할 때 비동기 로드하고, 테이블을 **행 구조체 이름**으로 등록한다. 조회는 `FindRow<FXxxTableRow>(RowName)`. 테이블을 추가할 때는 행 구조체를 만들고 `DA_GameData.mTables`에 넣기만 하면 된다(코드 수정 불필요).
- 로드는 비동기라서 BeginPlay 시점에 테이블이 없을 수 있다. 테이블을 읽는 초기화는 반드시 `CallOrRegisterDataLoaded(this, &T::Func)`를 거친다(캐릭터 `InitAttributes`, 스포너 `SpawnCharacter`).
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
- KayKit 메시는 임포트 때 만들어진 단순 콜리전이 메시 전체를 덮는 볼록 껍질(convex hull) 하나다. 그래서 문틀이나 성문처럼 구멍이 있는 메시는 구멍까지 막는다. 캐릭터 이동은 단순 콜리전을 쓰므로, 지나가야 하는 메시는 Collision Complexity를 `Use Complex Collision As Simple`로 바꾼다.
  - 2026-10-30에 바꾼 메시: `Dungeon/Assets/wall_doorway_scaffold`(Hospital / KyleHouse / Shop / Tavern, KyleHouse 방에서 나갈 수 없던 버그), `MedievalHexagon/.../Neutral/wall_straight_gate`·`wall_corner_A_gate`(Field의 `Gate_*` 5개, 성문을 지나갈 수 없던 버그).
  - `wall_doorway`(Palace / VoidRaindungeon)는 아직 그대로다. 실내 맵 출구 문틀은 그 안에 놓인 문 메시(`*_door`, 볼록 껍질 콜리전)가 막는다.
  - 확인: unreal-mcp `trace_world`는 막힌 구멍도 그냥 통과하므로 쓸 수 없다. 맵을 에디터에 열고 Simulate를 켠 채 `find_actors`(bounds + `ObjectTypeQuery1`) 겹침 검사를 한다(실제 PIE에서 레벨 이동한 뒤에는 `find_actors`가 PIE 월드를 보지 않는다). 겹침 검사는 삼각형 콜리전의 표면만 잡으므로, 상자가 벽 표면을 가로지르게 둔다.

### AI

- `AElmAIController`: AIPerception 시야 + `BT_Monster`/`BB_Monster`(`Content/AI`). 몬스터와 NPC가 같은 BT를 쓰고, 컨트롤러 팀은 조종하는 Pawn을 따른다.
- Blackboard 키 이름은 `MonsterBBKey` 네임스페이스와 BB 에셋이 정확히 같아야 한다.
- `BTService_CheckAttackRange`(0.1초)가 `UpdateTarget`으로 타겟을 고르고, 캡슐 표면 간 거리로 `InAttackRange`를 쓴다. `BTTask_MonsterAttack`은 `AAICharacter::PlayAttack`/`StopAttack`으로 어빌리티를 실행·취소한다.

### 레벨 이동 / 메모(SaveSubsystem)

- GameMode는 `DefaultEngine.ini`의 `GlobalDefaultGameMode`(`AElmGameModeBase`)라서 맵마다 지정하지 않는다. `ChoosePlayerStart`는 `APlayerStartPIE`("Play From Here") → 태그 없는 첫 PlayerStart → 엔진 기본(무작위) 순으로 고른다.
- `OpenLevel`은 GameInstance와 그 서브시스템 말고는 PlayerState(ASC), 캐릭터(가방)까지 모두 새로 만든다. 그래서 `USaveSubsystem`(GameInstanceSubsystem)이 "메모" `UElmSaveGame`(`Save/ElmSaveGame.h`)를 들고 있고, nullptr이면 새 게임이다.
  - 쓰기는 한 곳이다: `TravelToLevel` → `APlayerCharacter::WriteToSave`. 읽기는 부품마다 자기 초기화 때 한다(`UInventoryComponent::InitInventory`, `APlayerCharacter::InitAttributes`).
  - 이어 갈 상태를 늘리려면 `UElmSaveGame`에 필드를 추가하고 → `WriteToSave`에서 적고 → 그 부품의 초기화(`CallOrRegisterDataLoaded`를 거치는 함수)에서 읽는다.
  - 능력치는 Level / HP / MP / Exp / Gold의 Base 값만 적는다. 나머지는 Level로 `DT_LevelStat`에서 다시 계산된다. `InitLevelStats`(Max 값)보다 먼저 HP / MP를 넣으면 Clamp로 0이 된다.
- 이동: `ALevelPortal`(`Interaction/`, IInteractable)이 `USaveSubsystem::TravelToLevel(Player, Level, StartTag)`를 부른다. URL은 `"맵경로#태그"`이고, 엔진이 그 `PlayerStartTag`를 가진 PlayerStart에 세운다. 태그 = **연결 이름**: 한 문(연결)의 양쪽 끝이 같은 이름을 쓴다. Field ↔ Shop이면 Field 포탈과 Shop 포탈의 `mTargetStartTag`, Field와 Shop의 도착 PlayerStart 태그가 모두 `Shop`이다(`Shop#Shop`, `Field#Shop`). 같은 두 맵 사이에 문이 여러 개여도 구분된다. 예외는 어느 맵에서나 불리는 고정 태그다: `Respawn`(Hospital, 사망 후), `Room`(KyleHouse, 메뉴 "귀환"이 쓴다).
  - 지금 연결 이름: `OrcCave`, `Shop`, `Tavern`, `Hospital`, `HeroHouse`(KyleHouse), `Castle`(Palace). 실내 맵의 포탈은 `Exit_ToTown` 표식 앞 (x, 100, 105), Box Extent 100이다.
  - 페이드 아웃 / 인과 중복 이동 방지(`mIsTraveling` + 입력 끄기)도 `USaveSubsystem`이 맡는다. 도착은 `PostLoadMapWithWorld`로 알고, 로딩 직후 큰 DeltaTime 때문에 검은 화면을 0.5초 유지한 뒤 밝힌다. 카메라 페이드라 HUD(UMG)는 가려지지 않는다.
  - PIE 로그의 `UEDPIE_0_` 접두사 경로는 정상이다.
- 사망: `APlayerCharacter::OnKilledBy` → `AElmGameModeBase::PlayerKilled` → 2초 뒤 `TravelToLevel(Hospital, "Respawn")`. 메모의 HP가 0이면 죽어서 온 것으로 보고 `InitAttributes`가 HP / MP를 가득 찬 채로 둔다(부활 코드는 따로 없다). 사망 페널티는 없다.
- 디스크 저장: `USaveSubsystem::SaveToDisk` = `WritePlayer` + `SaveGameToSlot(mSaveGame, "ElmSave", 0)`(`Saved/SaveGames/ElmSave.sav`). 메뉴 "저장하기"만 부른다. 이동 중이거나 죽었으면 쓰지 않는다. 불러오기는 아직 없다(메모에 맵 / 위치가 없다).
- 결정 이유와 함정 기록은 끝나서 지운 진행 문서에 있다: `git show d3b7deb:Docs/LevelTravel_Progress.md`.

### 입력 / 상호작용 / UI

- 입력 설정은 `UElmInputConfig` 자식 하나가 IMC 하나와 그 IA들을 생성자에서 경로로 찾아 `FindInputAction("Name")`으로 제공한다: `UDefaultInput`(`Content/Input/Default/`, `IMC_Default`), `UDialogueInput`(`Content/Input/Dialogue/`, `IMC_Dialogue`, 대화 중에만 `IMC_Default` 대신 켠다). 입력을 추가하려면 해당 폴더에 IA를 만들고 → 그 입력 설정 생성자에 등록하고 → 캐릭터 동작은 `APlayerCharacter::SetupPlayerInputComponent`, UI/상호작용은 `AElmPlayerController::SetupInputComponent`에서 바인딩한다. 핸들러가 컨트롤러 컴포넌트의 함수여도 바인딩은 컨트롤러에서 한다(`mInteraction.Get()`처럼 원시 포인터로 넘긴다).
- 상호작용: `IInteractable`(C++ 전용 인터페이스)을 구현하고 해당 컴포넌트가 `Interaction` 채널에 Overlap으로 응답하면 된다. `UInteractionComponent`(`Player/`, `AElmPlayerController`에 붙음)가 0.1초마다 구 Overlap으로 가장 가까운 대상을 찾아 HUD에 `[F] ...` 문구를 띄운다. 키 글자는 `UElmInputConfig::FindMappedKeyText`가 지금 켜진 IMC에서 물어 온다. 다른 기능이 상호작용을 막을 때는 `SetBlocked(true / false)`를 부른다(상호작용 컴포넌트는 누가 막는지 모른다).
- 플레이어 컨트롤러에 기능을 더할 때는 `Player/`의 ActorComponent로 만들어 컨트롤러에 붙이고, 컨트롤러에는 생성과 키 바인딩만 둔다(2026-10-30 상호작용 / 대화를 이렇게 뺐다). 컴포넌트 `BeginPlay`는 컨트롤러 `BeginPlay`의 `Super` 안에서 불려 아직 HUD가 없다.
- HUD: `AElmPlayerController`가 `WBP_PlayerHUD`(`UPlayerHUDWidget`)를 만든다. C++ 위젯 변수는 `meta = (BindWidget)`이며 WBP의 위젯 이름과 같아야 한다. 상태바 같은 위젯은 `GetOwningPlayerState()`의 ASC Attribute 델리게이트를 구독해서 갱신한다. 창을 열고 닫을 때 입력 모드/커서는 `RefreshInputMode`가 맞춘다.
- 메뉴(B, `IA_Menu`): `AElmPlayerController::SetMenuOpen`이 `WBP_PlayerHUD`의 자식 `mMenuWidget`(`UMenuWidget` / `WBP_Menu`)을 켜고 `SetPause`로 게임을 멈춘다. 버튼은 일을 맡은 곳에 부탁만 한다: 귀환 → `AElmGameModeBase::Recall`(KyleHouse `Room`), 저장 → `USaveSubsystem::SaveToDisk`, 종료 → `QuitGame`(확인 상자). 귀환 / 저장은 살아 있고 전투 중이 아닐 때만 켠다(`AElmGameModeBase::IsInCombat` = 살아 있는 AI 중 Blackboard `Target`이 이 플레이어인 것이 있음).
  - 일시정지 중에는 **Trigger when Paused**를 켠 IA만 발동한다. `IA_Menu`만 켜 두어서 메뉴가 열린 동안 이동 / 공격 / I / F가 막힌다. 입력 모드는 `FInputModeGameAndUI`다(UIOnly면 B가 컨트롤러에 오지 않아 메뉴를 닫을 수 없다).
  - 일시정지 중에는 월드 타이머가 돌지 않는다. 타이머를 쓰는 일(`TravelToLevel`의 페이드)은 메뉴를 닫은 뒤에 부른다(`UMenuWidget::OnRecallClicked`).
  - 메뉴 버튼은 WBP에서 **Is Focusable**을 끈다. 켜 두면 클릭한 버튼이 포커스를 가져가 Space / Enter가 그 버튼을 다시 누른다.
- 창(인벤토리 I, 퀘스트 P)을 더하는 길: IA + `IMC_Default` 매핑 → `UDefaultInput` 등록 → 컨트롤러 바인딩 → `SetXxxOpen` → HUD `SetXxxVisible` / `IsXxxVisible` → `IsAnyWindowOpen`에 추가(→ `RefreshInputMode`가 커서와 GameAndUI를 맞춘다). IA는 빌드 전에 만들어 저장한다(입력 설정 CDO가 에디터 시작 때 경로로 찾는다).

### 대화 / 퀘스트

- 대화: `UDialogueComponent`(`Player/`, 컨트롤러에 붙음, `GetDialogue()`). NPC의 `Interact`가 시작 행을 골라 `StartDialogue(NPC, Row)`를 부른다. 대사는 `DT_Dialogue`(`FDialogueTableRow`: `Speaker` / `Text` / `NextRow`) 한 줄이 한 행이고 `NextRow`가 None이면 끝이다. 선택지는 없다.
  - 대화 중에는 `IMC_Default`를 빼고 `IMC_Dialogue`(F / Space / 마우스 왼쪽 → `IA_DialogueNext`)를 넣어 다른 입력을 한 번에 막는다. IMC를 바꾸는 순간 눌려 있던 키를 무시하는 기본 옵션은 Value Type이 Boolean인 IA에만 걸리므로 `IA_DialogueNext`는 Digital(bool)로 둔다. 커서는 띄우지 않는다.
  - 시작할 때 인벤토리 / 퀘스트 창을 닫고 `UInteractionComponent::SetBlocked(true)`를 부른다. NPC는 `BeginTalk`(BT `PauseLogic`, 플레이어 바라보기) / `EndTalk`(`ClearFocus` → `ResumeLogic`)로 멈췄다가 돌아간다. `EndTalk`는 NPC가 쓰러져 있어도 반드시 `ResumeLogic`을 부른다(사망 때의 `StopLogic`은 일시정지를 풀지 않아서, 안 풀면 부활 뒤에도 BT가 멈춰 있다).
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

## 코드 컨벤션

- 멤버 변수는 `m` 접두사(`mASC`, `mTeam`)를 쓰고, bool은 `b` 대신 `Is`를 쓴다(`mIsEssential`, `IsMagical`).
- 프로젝트 헤더는 상대 경로로 include하고(`"../GameInfo.h"`), 대부분의 헤더가 `GameInfo.h`를 먼저 include한다.
- 로그 카테고리: `LogTestDebug`(경고/에러/디버그), `LogTestOrder`(초기화 호출 순서 추적).
- 주석은 한글로, "왜 이렇게 하는지"를 적는다. 한 줄짜리 `if` 본문은 중괄호를 생략하는 스타일이다.
- 소스 파일은 UTF-8(BOM)로 저장한다. 2026-11-02 새로 만든 `QuestTrackerWidget.cpp`가 CP949(ANSI)로 저장되어, 한국어 Windows에서는 빌드되지만 git diff와 다른 도구에서는 한글이 깨졌다. VS에서는 파일 → 다른 이름으로 저장 → 저장 버튼 옆 ▼ → 인코딩하여 저장 → "유니코드(서명 있는 UTF-8) - 코드 페이지 65001"로 바꾼다. 한글이 든 `NSLOCTEXT` / `TEXT` 문자열이 있는 파일은 특히 지킨다.
- 생성자에서 에셋은 `ConstructorHelpers`로 전체 경로(`/Script/...'...'`)를 써서 찾는다. 클래스(ABP/WBP)는 경로 끝에 `_C`를 붙인다. 테이블로 정할 수 있는 외형/몽타주/어빌리티는 생성자 대신 DT의 Soft 참조를 쓴다.
