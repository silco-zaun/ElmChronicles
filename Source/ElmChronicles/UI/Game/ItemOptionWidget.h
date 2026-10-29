// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../UIInfo.h"
#include "Blueprint/UserWidget.h"
#include "ItemOptionWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UItemOptionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UItemOptionWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mOptionNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mOptionValueText;

protected:
	virtual void NativeOnInitialized();

protected:
	// 블루프린트에서 호출 가능한 함수를 만든다.
	UFUNCTION(BlueprintCallable)
	void SetData(UObject* EntryObject);
};
