// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckAttackRange.generated.h"

/**
 * Target이 AttackRange 안에 있는지 검사해서 Blackboard의 InAttackRange에 기록한다.
 */
UCLASS()
class ELMCHRONICLES_API UBTService_CheckAttackRange : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckAttackRange();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
