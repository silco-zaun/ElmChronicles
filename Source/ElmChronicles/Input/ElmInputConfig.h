// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "UObject/Object.h"
#include "ElmInputConfig.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UElmInputConfig : public UObject
{
	GENERATED_BODY()
	
public:
	UElmInputConfig();

public:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> mInputMappingContext;

protected:
	UPROPERTY()
	TMap<FString, TObjectPtr<UInputAction>> mInputActionMap;

public:
	TObjectPtr<UInputAction> FindInputAction(const FString& Name) const;
};
