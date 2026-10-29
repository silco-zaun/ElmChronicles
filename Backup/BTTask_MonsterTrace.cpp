// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MonsterTrace.h"
#include "../../Characters/MonsterCharacter.h"
#include "BTTask_MonsterAttack.h"

UBTTask_MonsterTrace::UBTTask_MonsterTrace()
{
	NodeName = TEXT("MonsterTrace");

	bNotifyTick = true;

	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MonsterTrace::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AIController를 얻어온다.
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
		return EBTNodeResult::Failed;

	// OwnerComp가 가지고 있는 BlackboardComponent를 얻어온다.
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
		return EBTNodeResult::Failed;

	// 블랙보드에서 Target을 얻어온다.
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));

	if (!Target)
		return EBTNodeResult::Failed;

	float AttackDist = BlackboardComp->GetValueAsFloat(TEXT("AttackDistance"));

	// 타겟을 향해 이동시킨다.
	// AIController는 NavAgent를 다중상속받아 구현되어 길찾기 기능을 이용할 수
	// 있다.
	EPathFollowingRequestResult::Type PathResult =
		AIController->MoveToActor(Target);

	if (PathResult == EPathFollowingRequestResult::Failed)
		return EBTNodeResult::Failed;

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (!Monster)
		return EBTNodeResult::Failed;

	Monster->ChangeAnim((uint8)EMonsterNormalAnimType::Run);

	// 도착할때까지 Task가 종료되면 안되기 때문에 계속 진행시킨다. 
	return EBTNodeResult::InProgress;
}

void UBTTask_MonsterTrace::TickTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, float DeltaSeconds)
{
	// AIController를 얻어온다.
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// OwnerComp가 가지고 있는 BlackboardComponent를 얻어온다.
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 블랙보드에서 Target을 얻어온다.
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));

	if (!Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 도착했는지 판단한다.
	EPathFollowingStatus::Type PathStatus =
		AIController->GetMoveStatus();

	if (PathStatus == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (!Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 곻격모션이 끝났을때 타겟과 몬스터의 거리를 구하여 공격거리 바깥으로
	// 빠져나갔을 경우 공격을 종료한다.
	FVector TargetLocation, MonsterLocation;

	TargetLocation = Target->GetActorLocation();

	UCapsuleComponent* Capsule =
		Cast<UCapsuleComponent>(Target->GetRootComponent());

	if (Capsule)
		TargetLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();

	MonsterLocation = Monster->GetActorLocation();

	Capsule = Cast<UCapsuleComponent>(Monster->GetRootComponent());

	if (Capsule)
		MonsterLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();

	// 두 점사이의거리를 구한다.
	float Distance = FVector::Dist(TargetLocation, MonsterLocation);

	float AttackDist = BlackboardComp->GetValueAsFloat(TEXT("AttackDistance"));

	if (Distance <= AttackDist)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
}


void UBTTask_MonsterTrace::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	AIController->StopMovement();
}