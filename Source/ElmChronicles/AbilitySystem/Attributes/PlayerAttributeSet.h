// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAttributeSet.h"
#include "PlayerAttributeSet.generated.h"

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
	ATTRIBUTE_FUNCTION(UPlayerAttributeSet, Job)
	ATTRIBUTE_FUNCTION(UPlayerAttributeSet, Exp)
	ATTRIBUTE_FUNCTION(UPlayerAttributeSet, Gold)
};
