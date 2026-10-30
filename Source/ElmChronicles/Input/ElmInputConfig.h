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

	// Name 액션에 지금 매핑된 키의 화면 글자("F"). 키를 아직 모르면 빈 FText를 돌려준다.
	// CDO는 모든 플레이어가 함께 쓰므로, 누구의 키인지는 LocalPlayer로 받는다.
	FText FindMappedKeyText(const ULocalPlayer* LocalPlayer, const FString& Name) const;
};
