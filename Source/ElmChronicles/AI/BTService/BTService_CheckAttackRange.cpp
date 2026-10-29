// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_CheckAttackRange.h"
#include "../ElmAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckAttackRange::UBTService_CheckAttackRange()
{
	NodeName = TEXT("Check Attack Range");

	Interval = 0.1f;
	RandomDeviation = 0.f;

	// 브랜치에 들어오는 순간에도 검사해서, 자식 Decorator가 최신 값으로 판단하게 한다.
	bCallTickOnSearchStart = true;
}

void UBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;

	if (!IsValid(Blackboard) || !IsValid(Pawn))
		return;

	// 타겟이 죽었거나 안 보이면 다른 적으로 바꾸거나 비운다.
	// 비면 Target 데코레이터가 공격 브랜치를 빠져나가고 BT가 HomeLocation으로 돌아간다.
	if (AElmAIController* MonsterController = Cast<AElmAIController>(AIController))
		MonsterController->UpdateTarget();

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(MonsterBBKey::Target));

	bool InRange = false;

	if (IsValid(Target))
	{
		// 중심 사이 거리에서 두 캡슐 반지름을 빼서 표면 사이 거리로 비교한다.
		float Distance = FVector::Dist2D(Pawn->GetActorLocation(), Target->GetActorLocation());
		Distance -= Pawn->GetSimpleCollisionRadius() + Target->GetSimpleCollisionRadius();

		InRange = Distance <= Blackboard->GetValueAsFloat(MonsterBBKey::AttackRange);
	}

	Blackboard->SetValueAsBool(MonsterBBKey::InAttackRange, InRange);
}
