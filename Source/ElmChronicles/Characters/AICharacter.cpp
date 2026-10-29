// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "../AbilitySystem/Abilities/GameplayAbility_Melee.h"
#include "../AbilitySystem/Attributes/AIAttributeSet.h"
#include "../AbilitySystem/Components/AIAbilitySystemComponent.h"
#include "../AI/ElmAIController.h"
#include "../Animations/AnimInstances/AIAnimInstance.h"
#include "../Data/Tables/LevelStatTableRow.h"
#include "Spawners/AISpawner.h"
#include "../Subsystem/AssetSubsystem.h"
#include "BrainComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AAICharacter::AAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	mASC = CreateDefaultSubobject<UAIAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AbilitySystemComponent를 가진 액터의 서브오브젝트로 만들면
	// AbilitySystemComponent가 초기화될 때 자동으로 등록된다.
	// 자식이 SetDefaultSubobjectClass로 "AttributeSet"의 클래스를 바꿔 두었으면 그 클래스로 만들어진다.
	mAttributeSet = CreateDefaultSubobject<UAIAttributeSet>(TEXT("AttributeSet"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AElmAIController::StaticClass();

	static ConstructorHelpers::FObjectFinder<UBehaviorTree>
		BTAsset(TEXT("/Script/AIModule.BehaviorTree'/Game/AI/BT_Monster.BT_Monster'"));

	if (BTAsset.Succeeded())
		mBehaviorTree = BTAsset.Object;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0, 360.0, 0.0);
	GetMesh()->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));
	// 캐릭터는 모두 Pawn이다. 아군 / 적 구분은 팀(mTeam)이 한다.
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

	// Owner와 Avatar가 모두 자기 자신이다.
	mASC->InitAbilityActorInfo(this, this);

	// 이동속도 / 피격 / 사망 델리게이트 연결. (부모 공통 처리)
	BindAttributeDelegates(mASC, mAttributeSet);

	// InitAttributes는 virtual이라, 몬스터라면 AMonsterCharacter::InitAttributes가 불린다.
	if (UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>())
		AssetSubsystem->CallOrRegisterDataLoaded(this, &AAICharacter::InitAttributes);
}

void AAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 죽어서 사라졌을 때(Destroy)만 스포너에 알린다. 레벨을 끝낼 때는 다시 만들 필요가 없다.
	// (필수 NPC는 Destroy되지 않으므로 다시 만들어지지 않는다)
	if (EndPlayReason == EEndPlayReason::Destroyed && IsValid(mSpawner))
		mSpawner->ResetSpawn();
}

UAbilitySystemComponent* AAICharacter::GetAbilitySystemComponent() const
{
	return mASC;
}

void AAICharacter::Death()
{
	Destroy();
}

void AAICharacter::InitAttributes()
{
	UAssetSubsystem* AssetSubsystem = GetGameInstance()->GetSubsystem<UAssetSubsystem>();
	// 몬스터면 DT_Monster, NPC면 DT_NPC에서 찾는다. (자식의 FindTableRow)
	const FAICharacterTableRow* Info = FindTableRow();

	if (!AssetSubsystem || !Info)
		return;

	// 외형은 눈에 보이는 부분이므로 서버와 클라이언트 모두 적용한다.
	InitAsset(*Info);

	// 능력치는 서버에서만 설정한다.
	if (!HasAuthority())
		return;

	// 공격 어빌리티 부여. 테이블에서 비워두면 근접 공격을 쓴다.
	// DT 열과 같은 타입(UElmGameplayAbility)으로 받는다. 더 좁은 타입으로 받으면 맞지 않는 클래스가 조용히 nullptr이 된다.
	TSubclassOf<UElmGameplayAbility> AbilityClass = Info->AttackAbility.LoadSynchronous();

	if (!AbilityClass)
		AbilityClass = UGameplayAbility_Melee::StaticClass();

	mAttackHandle = mASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));

	// 레벨에 해당하는 레벨 스탯을 찾는다. (DT_LevelStat 행 이름 = "1" ~ "10")
	const FLevelStatTableRow* Stat =
		AssetSubsystem->FindRow<FLevelStatTableRow>(FName(*FString::FromInt(Info->Level)));

	if (!Stat)
		return;

	// 레벨 스탯. Max 값을 먼저 넣는다.
	mASC->InitLevelStats(Info->Level, *Stat);
	mASC->InitCombatStats(Info->MoveSpeed, Info->AttackSpeed, Info->AttackRange, Info->AttackRadius);
	mASC->SetNumericAttributeBase(UAIAttributeSet::GetDetectRangeAttribute(), Info->DetectRange);

	// 감지 범위와 공격 거리를 AI에 전달한다.
	AElmAIController* AIController = GetController<AElmAIController>();

	if (IsValid(AIController))
	{
		AIController->SetDetectRange(Info->DetectRange);
		AIController->SetAttackRange(Info->AttackRange);
	}
}

void AAICharacter::OnDamagedBy(AActor* Causer, float Damage)
{
	UAIAnimInstance* Anim = Cast<UAIAnimInstance>(GetMesh()->GetAnimInstance());

	if (IsValid(Anim))
		Anim->PlayHit();
}

void AAICharacter::OnKilledBy(AActor* Killer)
{
	// State.Dead 태그와 어빌리티 취소는 부모(ElmCharacterBase)가 이미 했다.
	AAIController* AIController = GetController<AAIController>();

	if (IsValid(AIController))
	{
		if (AIController->BrainComponent)
			AIController->BrainComponent->StopLogic(TEXT("Death"));

		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AAICharacter::PlayAttack()
{
	// 실행에 성공해도 몽타주가 없으면 즉시 끝나므로, 실제로 진행 중인지까지 확인한다.
	return mASC->TryActivateAbility(mAttackHandle) && IsAttacking();
}

void AAICharacter::StopAttack()
{
	// 어빌리티를 취소하면 PlayMontageAndWait가 몽타주도 함께 멈춘다.
	mASC->CancelAbilityHandle(mAttackHandle);
}

bool AAICharacter::IsAttacking() const
{
	const FGameplayAbilitySpec* Spec = mASC->FindAbilitySpecFromHandle(mAttackHandle);

	return Spec && Spec->IsActive();
}
