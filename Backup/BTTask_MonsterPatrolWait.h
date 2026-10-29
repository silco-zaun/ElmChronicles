// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterPatrolWait.generated.h"

USTRUCT()
struct FWaitTimer
{
	GENERATED_BODY()

	FTimerHandle TimerHandle;
	bool Complete = false;
};

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UBTTask_MonsterPatrolWait : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MonsterPatrolWait();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float mWaitTime = 1.f;

public:
	virtual uint16 GetInstanceMemorySize() const;
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds);
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, EBTNodeResult::Type TaskResult);

	void WaitFinish(uint8* NodeMemory);
};
