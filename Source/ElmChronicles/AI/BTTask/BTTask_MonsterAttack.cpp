// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MonsterAttack.h"
#include "../ElmAIController.h"
#include "../../Characters/AICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MonsterAttack::UBTTask_MonsterAttack()
{
	NodeName = TEXT("Monster Attack");

	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AAICharacter* AICharacter = AIController ? AIController->GetPawn<AAICharacter>() : nullptr;

	if (!IsValid(AICharacter))
		return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(MonsterBBKey::Target));

	if (!IsValid(Target))
		return EBTNodeResult::Failed;

	// 제자리에서 타겟을 바라보며 공격한다.
	AIController->StopMovement();
	AIController->SetFocus(Target);

	if (!AICharacter->PlayAttack())
		return EBTNodeResult::Failed;   // 재생 실패 시 AttackEnd가 영원히 안 오므로 바로 실패 처리

	// 공격 몽타주가 끝날 때까지 TickTask에서 기다린다.
	return EBTNodeResult::InProgress;
}

void UBTTask_MonsterAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	AAICharacter* AICharacter = AIController ? AIController->GetPawn<AAICharacter>() : nullptr;

	if (!IsValid(AICharacter))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!AICharacter->IsAttacking())
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

void UBTTask_MonsterAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 정상 종료든 Abort(타겟 놓침, 사망)든 여기로 들어온다.
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (IsValid(AIController))
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);

		AAICharacter* AICharacter = AIController->GetPawn<AAICharacter>();

		if (IsValid(AICharacter))
		{
			if (TaskResult == EBTNodeResult::Aborted)
				AICharacter->StopAttack();
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
