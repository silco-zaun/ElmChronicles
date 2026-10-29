// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.generated.h"

#define ATTRIBUTE_FUNCTION(ClassName, AttributeName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, AttributeName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(AttributeName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(AttributeName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(AttributeName)

// 피격 / 사망 알림. Causer = 때린 캐릭터
DECLARE_MULTICAST_DELEGATE_TwoParams(FElmDamagedDelegate, AActor* /*Causer*/, float /*Damage*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FElmDeathDelegate, AActor* /*Killer*/);

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData PhysicalAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MagicalAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData PhysicalDefense;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MagicalDefense;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MoveSpeed;

	// 공격 속도 배율. 1.0이 원래 속도.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackSpeed;

	// 사거리 : 공격자 캡슐 표면 ~ 대상 캡슐 표면 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackRange;

	// 공격 판정 구의 반지름 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackRadius;

	// Meta Attribute : 이번에 받은 데미지를 잠깐 담는 칸. PostGameplayEffectExecute에서 HP로 옮기고 0으로 비운다.
	UPROPERTY(BlueprintReadOnly, Category = "Meta", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData IncomingDamage;

public:
	FElmDamagedDelegate mOnDamaged;   // 맞고 살아남았을 때
	FElmDeathDelegate mOnDeath;       // HP가 0이 된 순간 한 번

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, PhysicalAttack)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, MagicalAttack)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, PhysicalDefense)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, MagicalDefense)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, HP)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, MaxHP)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, MP)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, MaxMP)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, Level)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, MoveSpeed)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, AttackSpeed)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, AttackRange)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, AttackRadius)
	ATTRIBUTE_FUNCTION(UBaseAttributeSet, IncomingDamage)
};
