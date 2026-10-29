// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../UIInfo.h"
#include "UObject/Object.h"
#include "ItemOptionEntryObject.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UItemOptionEntryObject : public UObject
{
	GENERATED_BODY()
	
public:
	UItemOptionEntryObject();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString mOptionName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float mOption;

public:
	void SetOptionName(const FString& Name)
	{
		mOptionName = Name;
	}

	void SetOptionValue(float Value)
	{
		mOption = Value;
	}

public:
	const FString& GetOptionName() const
	{
		return mOptionName;
	}

	float GetOption() const
	{
		return mOption;
	}
};
