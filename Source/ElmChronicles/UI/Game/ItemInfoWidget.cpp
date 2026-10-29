// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInfoWidget.h"
#include "ItemOptionEntryObject.h"
#include "../../Data/Tables/ItemTableRow.h"



UItemInfoWidget::UItemInfoWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{

}

void UItemInfoWidget::SetItemInfo(const FItemTableRow& Info, int32 Upgrade)
{
	mNameText->SetText(FText::FromString(
		Info.Name + (Upgrade > 0 ? FString::Printf(TEXT(" + %d"), Upgrade) : TEXT(""))));
	mIconImage->SetBrushFromSoftTexture(Info.Icon);
	mLevelText->SetText(FText::FromString(FString::Printf(TEXT("Level : %d"), Info.Level)));
	mDescriptionText->SetText(FText::FromString(Info.Description));
	mBuyPriceText->SetText(FText::FromString(FString::Printf(TEXT("구매가격 : %d"), Info.BuyPrice)));
	mSellPriceText->SetText(FText::FromString(FString::Printf(TEXT("판매가격 : %d"), Info.SellPrice)));

	// 옵션 목록을 비워준다.
	mOptionList->ClearListItems();

	UE_LOG(LogTestDebug, Warning, TEXT("Option Num : %d"), Info.Options.Num());

	// 옵션 수만큼 반복한다.
	for (auto Value : Info.Options)
	{
		UItemOptionEntryObject* EntryObject = NewObject<UItemOptionEntryObject>();
		EntryObject->SetOptionName(StaticEnum<EItemOptionType>()->GetNameStringByValue((int64)Value.Type));
		EntryObject->SetOptionValue(Value.Value);

		mOptionList->AddItem(EntryObject);
	}
}

void UItemInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}
