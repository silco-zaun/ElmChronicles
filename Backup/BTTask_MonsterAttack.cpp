// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MonsterAttack.h"
#include "../../Characters/MonsterCharacter.h"


UBTTask_MonsterAttack::UBTTask_MonsterAttack()
{
	NodeName = TEXT("MonsterAttack");

	bNotifyTick = true;

	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(
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

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (!Monster)
		return EBTNodeResult::Failed;

	// 공격 애니메이션을 재생한다.
	Monster->ChangeAnim((uint8)EMonsterNormalAnimType::Attack);

	// 도착할때까지 Task가 종료되면 안되기 때문에 계속 진행시킨다.
	return EBTNodeResult::InProgress;
}

void UBTTask_MonsterAttack::TickTask(UBehaviorTreeComponent& OwnerComp,
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

	AMonsterCharacter* Monster = AIController->GetPawn<AMonsterCharacter>();

	if (!Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 공격 애니메이션을 재생한다.
	// Monster->ChangeAnim((uint8)EMonsterNormalAnimType::Attack);

	bool AttackEnd = BlackboardComp->GetValueAsBool(TEXT("AttackEnd"));

	if (AttackEnd)
	{
		BlackboardComp->SetValueAsBool(TEXT("AttackEnd"), false);

		// 공격모션이 끝났을때 타겟과 몬스터의 거리를 구하여 공격거리 바깥으로
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

		// 두 점 사이의 거리를 구한다.
		float Distance = FVector::Dist(TargetLocation, MonsterLocation);

		float AttackDist = BlackboardComp->GetValueAsFloat(TEXT("AttackDistance"));

		// 공격거리 바깥으로 빠져나갔을 경우
		if (Distance > AttackDist)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		// 여전히 공격거리 안쪽에 있을 경우
		else
		{
			FRotator Rot = GetTargetRotationYaw(TargetLocation,
				MonsterLocation);

			Monster->SetActorRotation(Rot);
		}
	}
}

void UBTTask_MonsterAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	AIController->StopMovement();
}