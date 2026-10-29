#pragma once

#include "../../GameInfo.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ElmDamageExecution.generated.h"

// 데미지 계산기. 공격자의 공격력과 맞는 쪽의 방어력을 읽어 HP를 깎는다.
UCLASS()
class ELMCHRONICLES_API UElmDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UElmDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
