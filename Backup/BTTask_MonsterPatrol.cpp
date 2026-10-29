// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MonsterPatrol.h"
#include "../../Characters/MonsterCharacter.h"

UBTTask_MonsterPatrol::UBTTask_MonsterPatrol()
{
	NodeName = TEXT("MonsterPatrol");

	// Tick을 호출할 수 있게 한다.
	bNotifyTick = true;

	// Task가 끝났을 때, OnTaskFinished를 호출할 수 있게 한다.
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MonsterPatrol::ExecuteTask(
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

	if (Target)
		return EBTNodeResult::Failed;

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (!Monster)
		return EBTNodeResult::Failed;

	// 순찰 지점으로 이동시킨다.
	EPathFollowingRequestResult::Type PathResult =
		AIController->MoveToLocation(Monster->GetPatrolPoint());

	if (PathResult == EPathFollowingRequestResult::Failed)
		return EBTNodeResult::Failed;

	Monster->ChangeAnim((uint8)EMonsterNormalAnimType::Walk);

	return EBTNodeResult::InProgress;
}

void UBTTask_MonsterPatrol::TickTask(UBehaviorTreeComponent& OwnerComp,
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

	if (Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (!Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	EPathFollowingStatus::Type PathStatus =
		AIController->GetMoveStatus();

	if (PathStatus == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector TargetLocation, MonsterLocation;

	TargetLocation = Monster->GetPatrolPoint();

	MonsterLocation = Monster->GetActorLocation();

	UCapsuleComponent* Capsule =
		Cast<UCapsuleComponent>(Monster->GetRootComponent());

	// 두 점 사이의 거리를 구한다.
	float Distance = FVector::Dist(TargetLocation, MonsterLocation);

	if (Distance <= 5.f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}

}

void UBTTask_MonsterPatrol::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	AIController->StopMovement();

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (Monster)
	{
		if (Monster->GetPatrolEnable())
			Monster->NextPatrol();
	}
}
