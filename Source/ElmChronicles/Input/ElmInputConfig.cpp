// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmInputConfig.h"

UElmInputConfig::UElmInputConfig()
{
}

TObjectPtr<UInputAction> UElmInputConfig::FindInputAction(const FString& Name) const
{
	return mInputActionMap.FindRef(Name);
}
