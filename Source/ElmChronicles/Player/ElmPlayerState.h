// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "ElmPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API AElmPlayerState : public APlayerState,
	public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AElmPlayerState();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UPlayerAbilitySystemComponent> mASC;

	UPROPERTY()
	TObjectPtr<class UPlayerAttributeSet> mAttributeSet;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPlayerAbilitySystemComponent* GetPlayerAbilitySystemComponent() const
	{
		return mASC;
	}

	UPlayerAttributeSet* GetAttributeSet() const
	{
		return mAttributeSet;
	}
};
