// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MonsterPatrolWait.h"
#include "../../Characters/MonsterCharacter.h"

UBTTask_MonsterPatrolWait::UBTTask_MonsterPatrolWait()
{
	NodeName = TEXT("MonsterPatrolWait");

	// Tick을 호출할 수 있게 한다.
	bNotifyTick = true;

	// Task가 끝났을 때, OnTaskFinished를 호출할 수 있게 한다.
	bNotifyTaskFinished = true;
}

uint16 UBTTask_MonsterPatrolWait::GetInstanceMemorySize() const
{
	return (uint16)sizeof(FWaitTimer);
}

EBTNodeResult::Type UBTTask_MonsterPatrolWait::ExecuteTask(
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

	Monster->ChangeAnim((uint8)EMonsterNormalAnimType::Idle);

	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;

	Timer->Complete = false;

	OwnerComp.GetWorld()->GetTimerManager().SetTimer(Timer->TimerHandle,
		FTimerDelegate::CreateUObject(this, &UBTTask_MonsterPatrolWait::WaitFinish,
			NodeMemory), mWaitTime, false);

	// 도착할때까지 Task가 종료되면 안되기 때문에 계속 진행시킨다.
	return EBTNodeResult::InProgress;
}

void UBTTask_MonsterPatrolWait::TickTask(UBehaviorTreeComponent& OwnerComp,
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

	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;

	if (Timer->Complete)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
}

void UBTTask_MonsterPatrolWait::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;

	if (Timer->TimerHandle.IsValid())
	{
		OwnerComp.GetWorld()->GetTimerManager().ClearTimer(Timer->TimerHandle);
	}
}

void UBTTask_MonsterPatrolWait::WaitFinish(uint8* NodeMemory)
{
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;

	Timer->Complete = true;
}
