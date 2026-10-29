// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffect_Damage.h"
#include "ElmDamageExecution.h"


UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// HP를 얼마나 깎을지는 계산기가 맞는 순간에 정한다.
	FGameplayEffectExecutionDefinition Execution;
	Execution.CalculationClass = UElmDamageExecution::StaticClass();

	Executions.Add(Execution);
}
