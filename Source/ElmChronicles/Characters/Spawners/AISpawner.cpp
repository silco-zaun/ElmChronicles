// Fill out your copyright notice in the Description page of Project Settings.


#include "AISpawner.h"
#include "../AICharacter.h"
#include "../MonsterCharacter.h"
#include "../NPCCharacter.h"
#include "../../Data/Tables/MonsterCharacterTableRow.h"
#include "../../Data/Tables/NPCCharacterTableRow.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAISpawner::AAISpawner()
{
	// 스포너는 타이머로만 동작하므로 Tick이 필요 없다.
	PrimaryActorTick.bCanEverTick = false;

	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	mPatrolPath = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolPath"));

	mPatrolPath->SetupAttachment(mRoot);

#if WITH_EDITORONLY_DATA
	// 에디터에서는 RootComponent가 어디에 있는지 표시해준다.
	mRoot->bVisualizeComponent = true;

	mArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	// 에디터 전용 컴포넌트라 게임 실행 환경에서는 만들어지지 않을 수 있다.
	// 색은 종류에 따라 달라지므로 OnConstruction에서 정한다.
	if (mArrow)
	{
		mArrow->bTreatAsASprite = true;
		mArrow->bIsScreenSizeScaled = true;
		mArrow->SetupAttachment(mRoot);
	}
#endif

	SetRootComponent(mRoot);
}

TArray<FString> AAISpawner::GetRowNameOptions() const
{
	// 고른 종류의 테이블 행만 보여준다.
	const UScriptStruct* RowStruct = mSpawnKind == EAISpawnKind::NPC
		? FNPCCharacterTableRow::StaticStruct()
		: FMonsterCharacterTableRow::StaticStruct();

	return UAssetSubsystem::GetRowNamesInEditor(RowStruct);
}

TSubclassOf<AAICharacter> AAISpawner::GetSpawnClass() const
{
	if (mSpawnKind == EAISpawnKind::NPC)
		return ANPCCharacter::StaticClass();

	return AMonsterCharacter::StaticClass();
}

float AAISpawner::GetSpawnHalfHeight() const
{
	if (mSpawnKind == EAISpawnKind::NPC)
	{
		const FNPCCharacterTableRow* Info = FindDataRow<FNPCCharacterTableRow>();

		return Info ? Info->CapsuleHalfHeight : -1.f;
	}

	const FMonsterCharacterTableRow* Info = FindDataRow<FMonsterCharacterTableRow>();

	// Epic / Boss는 몸이 커지므로(ApplyTypeScale) 캡슐 높이에도 같은 배율을 곱한다.
	return Info ? Info->CapsuleHalfHeight * GetMonsterTypeSizeScale(Info->Type) : -1.f;
}

void AAISpawner::BeginPlay()
{
	Super::BeginPlay();

	// 테이블이 로드된 뒤에 스폰해야 행을 찾을 수 있다.
	if (UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>())
		AssetSubsystem->CallOrRegisterDataLoaded(this, &AAISpawner::SpawnCharacter);
}

void AAISpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	// 몬스터는 빨강, NPC는 파랑 화살표로 그려 뷰포트에서 구분한다.
	if (mArrow)
		mArrow->SetArrowFColor(mSpawnKind == EAISpawnKind::NPC ? FColor(100, 180, 255) : FColor(255, 120, 130));
#endif

	// 배열을 비워준다.
	mPatrolPoints.Empty();

	int32 Count = mPatrolPath->GetNumberOfSplinePoints();

	for (int32 i = 0; i < Count; ++i)
	{
		FVector Point = mPatrolPath->GetLocationAtSplinePoint(i,
			ESplineCoordinateSpace::World);

		mPatrolPoints.Add(Point);
	}
}

#if WITH_EDITOR
void AAISpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// 종류를 바꾸면 그 종류의 기본값으로 맞춘다.
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(AAISpawner, mSpawnKind))
	{
		// 이전 테이블의 행 이름은 새 테이블에 없으므로 비운다. (콤보박스에서 다시 고른다)
		mRowName = NAME_None;

		// 주민이 죽으면 30초 뒤 다시 나온다. (몬스터가 계속 쳐들어와도 마을이 비지 않게)
		// 필수 NPC는 사라지지 않고 쓰러졌다 일어나므로 이 값과 상관없다.
		mSpawnDelay = mSpawnKind == EAISpawnKind::NPC ? 30.f : 0.f;
	}

	// 부모가 OnConstruction을 다시 실행하므로(화살표 색) 값을 다 바꾼 뒤에 부른다.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AAISpawner::ResetSpawn()
{
	if (mSpawnType == EMonsterSpawnType::Once)
	{
		Destroy();
		return;
	}

	if (mSpawnDelay <= 0.f)
		SpawnCharacter();
	else
	{
		// 타이머를 생성한다.
		GetWorldTimerManager().SetTimer(
			mSpawnTimerHandle, this, &AAISpawner::SpawnTimerCallback,
			mSpawnDelay, false);
	}
}

void AAISpawner::SpawnTimerCallback()
{
	GetWorldTimerManager().ClearTimer(mSpawnTimerHandle);

	SpawnCharacter();
}

void AAISpawner::SpawnCharacter()
{
	const TSubclassOf<AAICharacter> SpawnClass = GetSpawnClass();
	// 행을 찾지 못하면(테이블 미등록, 행 이름 오타) 음수가 온다.
	const float HalfHeight = GetSpawnHalfHeight();

	if (!SpawnClass || HalfHeight < 0.f)
	{
		UE_LOG(LogTestDebug, Warning, TEXT("%s : 스폰 실패 (행 : %s)"), *GetName(), *mRowName.ToString());
		return;
	}

	// 스포너는 바닥에 놓으므로 캡슐 중심이 바닥에서 HalfHeight만큼 위에 오게 만든다.
	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z += HalfHeight;

	const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

	// 액터만 만들고 BeginPlay는 FinishSpawning까지 미룬다.
	AAICharacter* Character = GetWorld()->SpawnActorDeferred<AAICharacter>(
		SpawnClass, SpawnTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!IsValid(Character))
		return;

	// BeginPlay 전에 필요한 정보를 모두 넣는다.
	Character->SetRowName(mRowName);
	Character->SetSpawner(this);
	Character->SetPatrolPoints(mPatrolPoints);

	// 이 안에서 BeginPlay → InitAttributes → InitAsset이 실행된다.
	Character->FinishSpawning(SpawnTransform);

	mSpawnedCharacter = Character;
}
