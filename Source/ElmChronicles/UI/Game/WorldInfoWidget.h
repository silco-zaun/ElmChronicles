// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../UIInfo.h"
#include "Blueprint/UserWidget.h"
#include "WorldInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UWorldInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UWorldInfoWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> mHPBar;

protected:
	virtual void NativeOnInitialized();

public:
	void SetInfoName(const FString& Name);
	void SetHP(float HP, float HPMax);
};
