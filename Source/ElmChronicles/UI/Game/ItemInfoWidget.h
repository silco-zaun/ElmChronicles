// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../UIInfo.h"
#include "../../Data/Tables/ItemTableRow.h"
#include "Blueprint/UserWidget.h"
#include "ItemInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UItemInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UItemInfoWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> mIconImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mLevelText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mDescriptionText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mBuyPriceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mSellPriceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UListView> mOptionList;

public:
	void SetItemInfo(const FItemTableRow& Info, int32 Upgrade);

protected:
	virtual void NativeOnInitialized();
};
