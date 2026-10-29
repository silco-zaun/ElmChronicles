// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "ElmAIController.generated.h"

// Blackboard 키 이름. BB_Monster 에셋의 키 이름과 똑같아야 한다.
namespace MonsterBBKey
{
	inline const FName Target(TEXT("Target"));
	inline const FName AttackRange(TEXT("AttackRange"));
	inline const FName InAttackRange(TEXT("InAttackRange"));
	inline const FName HomeLocation(TEXT("HomeLocation"));
}

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API AElmAIController : public AAIController
{
	GENERATED_BODY()

public:
	AElmAIController();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElmAIController")
	TObjectPtr<UAIPerceptionComponent> mPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElmAIController")
	TObjectPtr<class UAISenseConfig_Sight> mSightConfig;

	// 시야 반경보다 이만큼 더 멀어져야 타겟을 놓친다.
	static constexpr float LoseSightMargin = 100.f;

public:
	void SetDetectRange(float Range);
	void SetAttackRange(float Range);
	// 대상의 ASC에 State.Dead 태그가 있으면 죽은 것으로 본다.
	static bool IsDeadTarget(const AActor* Target);
	// Blackboard의 Target. 싸우는 상대가 없으면 nullptr.
	AActor* GetTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	// 팀은 조종하는 Pawn을 따른다. (몬스터 = Monster, NPC = NPC)
	virtual FGenericTeamId GetGenericTeamId() const override;

public:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 지금 타겟을 유지할지, 보이는 적 중 가장 가까운 대상으로 바꿀지 정해서 Blackboard에 쓴다.
	// 시야가 바뀔 때와 BTService_CheckAttackRange가 0.1초마다 부른다.
	void UpdateTarget();
};
