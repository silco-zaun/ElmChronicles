#include "ElmDamageExecution.h"
#include "../ElmGameplayTags.h"
#include "../Attributes/BaseAttributeSet.h"

namespace
{
	// 어떤 Attribute를 누구에게서 읽을지 정의한다. (Source = 공격자, Target = 맞는 쪽)
	// Snapshot true  : 스펙을 만든 순간의 값을 쓴다. (발사 시점의 공격력)
	// Snapshot false : 적용되는 순간의 값을 쓴다. (맞는 순간의 방어력)
	struct FDamageCaptures
	{
		FGameplayEffectAttributeCaptureDefinition PhysicalAttack{
			UBaseAttributeSet::GetPhysicalAttackAttribute(), EGameplayEffectAttributeCaptureSource::Source, true };
		FGameplayEffectAttributeCaptureDefinition MagicalAttack{
			UBaseAttributeSet::GetMagicalAttackAttribute(), EGameplayEffectAttributeCaptureSource::Source, true };
		FGameplayEffectAttributeCaptureDefinition PhysicalDefense{
			UBaseAttributeSet::GetPhysicalDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false };
		FGameplayEffectAttributeCaptureDefinition MagicalDefense{
			UBaseAttributeSet::GetMagicalDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false };
	};

	const FDamageCaptures& GetDamageCaptures()
	{
		static const FDamageCaptures Captures;
		return Captures;
	}
}

UElmDamageExecution::UElmDamageExecution()
{
	const FDamageCaptures& Captures = GetDamageCaptures();

	// 여기 등록한 Attribute만 Execute에서 읽을 수 있다.
	RelevantAttributesToCapture.Add(Captures.PhysicalAttack);
	RelevantAttributesToCapture.Add(Captures.MagicalAttack);
	RelevantAttributesToCapture.Add(Captures.PhysicalDefense);
	RelevantAttributesToCapture.Add(Captures.MagicalDefense);
}

void UElmDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FDamageCaptures& Captures = GetDamageCaptures();

	// 어빌리티가 Damage.Magical 태그를 붙였으면 마법 공식으로 계산한다.
	const bool IsMagical = Spec.GetDynamicAssetTags().HasTag(ElmTags::Damage_Magical);

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackStat = 0.f;
	float DefenseStat = 0.f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		IsMagical ? Captures.MagicalAttack : Captures.PhysicalAttack, EvalParams, AttackStat);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		IsMagical ? Captures.MagicalDefense : Captures.PhysicalDefense, EvalParams, DefenseStat);

	// 데미지 공식
	const float Damage = FMath::Max(1.f, (AttackStat * 100.f) / (100.f + DefenseStat));

	// HP를 직접 깎지 않고 IncomingDamage에 넣는다. HP 차감은 AttributeSet이 한다.
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UBaseAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage));
}
