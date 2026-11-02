// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCCharacter.h"
#include "../AbilitySystem/Attributes/BaseAttributeSet.h"
#include "../AbilitySystem/Attributes/NPCAttributeSet.h"
#include "../AbilitySystem/Components/AIAbilitySystemComponent.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "../AI/ElmAIController.h"
#include "../Data/Tables/QuestTableRow.h"
#include "../ElmCollisionChannels.h"
#include "../Player/DialogueComponent.h"
#include "../Player/ElmPlayerController.h"
#include "../Player/QuestComponent.h"
#include "PlayerCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// 부모 생성자가 "AttributeSet"을 만들기 전에 그 클래스를 UNPCAttributeSet으로 바꿔 둔다. (AMonsterCharacter와 같은 방식)
ANPCCharacter::ANPCCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNPCAttributeSet>(TEXT("AttributeSet")))
{
	// 누구와 싸울지는 TeamTypes.h의 관계표가 정한다. (기본: 플레이어와 중립, 몬스터와 적대)
	mTeam = ETeamType::NPC;

	// 레벨에 배치하면 이 행으로 시작한다. 인스턴스마다 Details 패널에서 바꿀 수 있다.
	mRowName = TEXT("Villager");

	// 에디터 뷰포트 미리보기용. 플레이하면 InitAsset이 테이블 값(BodyMesh, 캡슐)으로 다시 정한다.
	// (몬스터는 스포너가 만들어서 필요 없지만, NPC는 레벨에 직접 배치하므로 보여야 놓기 쉽다)
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PreviewMesh(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/KayKit/Adventurers/Characters/Rogue/SkeletalMeshes/Rogue.Rogue'"));

	if (PreviewMesh.Succeeded())
		GetMesh()->SetSkeletalMeshAsset(PreviewMesh.Object);

	GetCapsuleComponent()->SetCapsuleSize(55.f, 125.f);
	GetMesh()->SetRelativeLocation(FVector(0.0, 0.0, -125.0));

	// 플레이어의 상호작용 검색(Interaction 채널)에 걸리게 한다. 나머지 채널은 Pawn 프로필 그대로다.
	// (부모 생성자의 SetCollisionProfileName("Pawn") 뒤에 실행되어야 하므로 여기서 한다)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ElmChannels::Interaction, ECR_Overlap);
}

void ANPCCharacter::InitAttributes()
{
	// 외형 / 능력치 / 공격 / AI 설정은 부모가 한다.
	Super::InitAttributes();

	const FNPCCharacterTableRow* Info = FindTableRow();

	if (!Info)
		return;

	mIsEssential = Info->IsEssential;
	mReviveTime = Info->ReviveTime;
	mDialogueRow = Info->DialogueRow;
	mQuestRow = Info->QuestRow;
}

const FNPCCharacterTableRow* ANPCCharacter::FindTableRow() const
{
	return FindDataRow<FNPCCharacterTableRow>();
}

void ANPCCharacter::OnKilledBy(AActor* Killer)
{
	// AI 정지와 충돌 끄기는 부모가 한다.
	Super::OnKilledBy(Killer);

	// 필수 NPC는 잠시 뒤 일어난다. (일반 NPC는 사망 애니메이션이 끝나면 Death()에서 사라진다)
	if (mIsEssential)
		GetWorldTimerManager().SetTimer(mReviveTimer, this, &ANPCCharacter::Revive, mReviveTime, false);
}

void ANPCCharacter::Death()
{
	// 필수 NPC는 사라지지 않고 쓰러진 채로 Revive를 기다린다.
	if (mIsEssential)
		return;

	Super::Death();
}

FText ANPCCharacter::GetInteractText() const
{
	// 화면에 보이는 고정 문구는 NSLOCTEXT로 만든다. (나중에 번역 도구가 찾을 수 있다)
	return NSLOCTEXT("ElmNPC", "Talk", "대화하기");
}

bool ANPCCharacter::CanInteract(const APlayerCharacter* Player) const
{
	// 대화가 없는 NPC(DialogueRow = None)이거나 쓰러져 있으면 말을 걸 수 없다.
	if (mDialogueRow.IsNone() || IsDead())
		return false;

	// 싸우는 중(Blackboard에 Target이 있음)에도 말을 걸 수 없다.
	const AElmAIController* AIController = GetController<AElmAIController>();

	return !IsValid(AIController) || !IsValid(AIController->GetTarget());
}

void ANPCCharacter::Interact(APlayerCharacter* Player)
{
	// 대화 진행(대화창, 입력 바꾸기, 다음 줄)은 플레이어 컨트롤러의 대화 컴포넌트가 맡는다. NPC는 어느 행에서 시작할지만 알려 준다.
	AElmPlayerController* PlayerController = Player->GetController<AElmPlayerController>();

	if (IsValid(PlayerController))
		PlayerController->GetDialogue()->StartDialogue(this, ChooseDialogueRow(PlayerController->GetQuest()));
}

FName ANPCCharacter::ChooseDialogueRow(const UQuestComponent* Quest) const
{
	// 퀘스트가 없는 NPC(QuestRow = None)이면 FindQuestInfo가 nullptr을 돌려준다.
	const FQuestTableRow* QuestInfo = Quest->FindQuestInfo(mQuestRow);

	if (!QuestInfo)
		return mDialogueRow;

	// 받기 전에는 평소 대사(의뢰 내용)로 시작하고, 그 대화를 끝까지 들으면 퀘스트를 받는다.
	// 받은 뒤에는 같은 의뢰를 다시 듣지 않도록 진행 상태에 맞는 대사로 시작한다. 그 대사를 비워 두면 평소 대사를 쓴다.
	switch (Quest->GetQuestState(mQuestRow))
	{
	case EQuestState::InProgress:
		return QuestInfo->ProgressDialogue.IsNone() ? mDialogueRow : QuestInfo->ProgressDialogue;
	case EQuestState::Completed:
		return QuestInfo->CompleteDialogue.IsNone() ? mDialogueRow : QuestInfo->CompleteDialogue;
	default:
		return mDialogueRow;
	}
}

void ANPCCharacter::BeginTalk(APawn* Listener)
{
	AAIController* AIController = GetController<AAIController>();

	if (!IsValid(AIController))
		return;

	// BT를 멈춘다. 그대로 두면 순찰이나 귀가(HomeLocation) 중인 NPC가 말하다가 걸어가 버린다.
	// 멈춘 동안에도 시야(Perception)는 Blackboard의 Target을 갱신하므로, 적이 보이면 CanInteract가 false가 되어 대화가 끊긴다.
	if (AIController->BrainComponent)
		AIController->BrainComponent->PauseLogic(TEXT("Talk"));

	// BT가 멈춰도 이미 시작한 이동은 계속되므로 따로 세운다.
	AIController->StopMovement();
	// 말하는 상대 쪽으로 돈다. (bUseControllerDesiredRotation이라 RotationRate 속도로 천천히 돈다)
	AIController->SetFocus(Listener);
}

void ANPCCharacter::EndTalk()
{
	AAIController* AIController = GetController<AAIController>();

	if (!IsValid(AIController))
		return;

	// 바라보기를 먼저 푼다. BT가 다시 돌면 공격 태스크가 같은 우선순위(Gameplay)로 포커스를 걸기 때문이다.
	AIController->ClearFocus(EAIFocusPriority::Gameplay);

	// 쓰러져 있어도 반드시 푼다. 사망 때의 StopLogic은 일시정지를 풀지 않아서, 여기서 안 풀면 부활(RestartLogic) 뒤에도 BT가 멈춰 있다.
	// (멈춘 BT에 ResumeLogic을 불러도 실행할 노드가 없어서 다시 돌지는 않는다)
	if (AIController->BrainComponent)
		AIController->BrainComponent->ResumeLogic(TEXT("Talk"));
}

void ANPCCharacter::Revive()
{
	// HP를 먼저 채운다. (State.Dead를 먼저 지우면 잠깐 동안 "HP 0인 산 캐릭터"가 된다)
	const float MaxHP = mASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute());
	mASC->SetNumericAttributeBase(UBaseAttributeSet::GetHPAttribute(), MaxHP * ReviveHPRatio);

	// 죽음 상태를 푼다. 어빌리티를 다시 쓸 수 있고, 애니메이션이 Idle로 돌아가고, 몬스터가 다시 타겟으로 고른다.
	mASC->RemoveLooseGameplayTag(ElmTags::State_Dead);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 부모 OnKilledBy에서 멈췄던 BehaviorTree를 다시 돌린다.
	AAIController* AIController = GetController<AAIController>();

	if (IsValid(AIController) && AIController->BrainComponent)
		AIController->BrainComponent->RestartLogic();
}
