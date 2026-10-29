// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterPatrol.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UBTTask_MonsterPatrol : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MonsterPatrol();

protected:

public:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds);
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, EBTNodeResult::Type TaskResult);
};
