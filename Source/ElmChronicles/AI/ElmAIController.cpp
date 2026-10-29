// Fill out your copyright notice in the Description page of Project Settings.

#include "ElmAIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "../AbilitySystem/ElmGameplayTags.h"
#include "../Characters/AICharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TeamTypes.h"

AElmAIController::AElmAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	mSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	mSightConfig->SightRadius = 500.f;
	mSightConfig->LoseSightRadius = mSightConfig->SightRadius + LoseSightMargin;
	mSightConfig->PeripheralVisionAngleDegrees = 180.f; // 360도 전방위 시야
	mSightConfig->SetMaxAge(5.f);
	mSightConfig->DetectionByAffiliation.bDetectEnemies = true;
	mSightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	mSightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	mPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	mPerception->ConfigureSense(*mSightConfig);
	mPerception->SetDominantSense(mSightConfig->GetSenseImplementation());
	mPerception->OnTargetPerceptionUpdated.AddDynamic(this,	&AElmAIController::OnTargetPerceptionUpdated);

	SetPerceptionComponent(*mPerception);
}

void AElmAIController::SetDetectRange(float Range)
{
	mSightConfig->SightRadius = Range;
	mSightConfig->LoseSightRadius = Range + LoseSightMargin;

	// 바뀐 설정을 Perception에 다시 적용한다.
	mPerception->ConfigureSense(*mSightConfig);
}

void AElmAIController::SetAttackRange(float Range)
{
	if (!IsValid(Blackboard))
	{
		UE_LOG(LogTestDebug, Warning, TEXT("SetAttackRange : Invalid Blackboard"));

		return;
	}

	Blackboard->SetValueAsFloat(MonsterBBKey::AttackRange, Range);
}

bool AElmAIController::IsDeadTarget(const AActor* Target)
{
	// IAbilitySystemInterface를 통해 ASC를 찾는다. 플레이어는 PlayerState의 ASC가 나온다.
	const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);

	return TargetASC && TargetASC->HasMatchingGameplayTag(ElmTags::State_Dead);
}

AActor* AElmAIController::GetTarget() const
{
	if (!IsValid(Blackboard))
		return nullptr;

	return Cast<AActor>(Blackboard->GetValueAsObject(MonsterBBKey::Target));
}

void AElmAIController::BeginPlay()
{
	Super::BeginPlay();

	// 시작할 때 강제로 주변을 탐색하게 갱신한다.
	mPerception->RequestStimuliListenerUpdate();
}

void AElmAIController::OnPossess(APawn* InPawn)
{
	// Super가 끝나야 GetPawn()이 유효하다.
	Super::OnPossess(InPawn);

	// 팀이 Pawn에서 오므로, Pawn이 생긴 지금 시야에 팀을 다시 알려준다.
	// (스포너가 만든 몬스터는 컨트롤러 BeginPlay가 빙의보다 먼저라서 그때는 팀이 없다)
	mPerception->RequestStimuliListenerUpdate();

	AAICharacter* AICharacter = Cast<AAICharacter>(InPawn);

	if (IsValid(AICharacter))
	{
		// BehaviorTree를 실행한다. BT에 연결된 Blackboard도 이때 만들어진다.
		if (!RunBehaviorTree(AICharacter->GetBehaviorTree()))
		{
			UE_LOG(LogTestDebug, Warning, TEXT("OnPossess : RunBehaviorTree failed (%s)"), *GetNameSafe(AICharacter));
			return;
		}

		// 처음 자리(스폰 지점 / 레벨에 배치한 자리). 싸울 상대가 없으면 여기로 돌아간다.
		Blackboard->SetValueAsVector(MonsterBBKey::HomeLocation, AICharacter->GetActorLocation());
	}
}

FGenericTeamId AElmAIController::GetGenericTeamId() const
{
	// 시야(Perception)는 컨트롤러의 팀으로 적을 고른다. Pawn의 팀을 그대로 돌려준다.
	return FGenericTeamId::GetTeamIdentifier(GetPawn());
}

void AElmAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 누가 보이거나 사라질 때마다 타겟을 다시 고른다. (보이는 적 목록은 Perception이 들고 있다)
	UpdateTarget();
}

void AElmAIController::UpdateTarget()
{
	APawn* MyPawn = GetPawn();

	if (!IsValid(Blackboard) || !IsValid(MyPawn))
		return;

	// 지금 시야 안에 있는 액터들. (Sight의 DetectionByAffiliation 설정대로 적만 들어온다)
	TArray<AActor*> PerceivedActors;
	mPerception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	AActor* CurrentTarget = GetTarget();

	// 지금 타겟이 살아 있고 아직 보이면 그대로 둔다.
	// (매번 가장 가까운 적으로 바꾸면 여럿이 섞여 싸울 때 타겟이 계속 흔들린다)
	if (IsValid(CurrentTarget) && !IsDeadTarget(CurrentTarget) && PerceivedActors.Contains(CurrentTarget))
		return;

	// 보이는 적 중 살아 있는 가장 가까운 대상을 고른다. 없으면 nullptr → BT가 HomeLocation으로 돌아간다.
	AActor* NewTarget = nullptr;
	double NearestDistSq = TNumericLimits<double>::Max();

	for (AActor* Actor : PerceivedActors)
	{
		if (!IsValid(Actor) || IsDeadTarget(Actor))
			continue;

		// 데미지는 적대에게만 들어가므로, 공격 대상도 적대만 고른다.
		if (!IsHostile(MyPawn, Actor))
			continue;

		const double DistSq = FVector::DistSquared(MyPawn->GetActorLocation(), Actor->GetActorLocation());

		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NewTarget = Actor;
		}
	}

	if (NewTarget != CurrentTarget)
	{
		UE_LOG(LogTestDebug, Log, TEXT("%s Target : %s"), *GetNameSafe(MyPawn), *GetNameSafe(NewTarget));

		Blackboard->SetValueAsObject(MonsterBBKey::Target, NewTarget);
	}
}
