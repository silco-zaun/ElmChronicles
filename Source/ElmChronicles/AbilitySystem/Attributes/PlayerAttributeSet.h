// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAttributeSet.h"
#include "PlayerAttributeSet.generated.h"

// 경험치를 GE(몬스터 / 퀘스트 보상)로 받았을 때 알린다. 레벨업 판단은 DT_LevelStat을 읽는 캐릭터가 한다.
DECLARE_MULTICAST_DELEGATE(FElmExpGainedDelegate);

/**
 *
 */
UCLASS()
class ELMCHRONICLES_API UPlayerAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Job;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData	Exp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerAttributeSet", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData	Gold;

public:
	FElmExpGainedDelegate mOnExpGained;

public:
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	ATTRIBUTE_FUNCTION(UPlayerAttributeSet, Job)
		ATTRIBUTE_FUNCTION(UPlayerAttributeSet, Exp)
		ATTRIBUTE_FUNCTION(UPlayerAttributeSet, Gold)
};
