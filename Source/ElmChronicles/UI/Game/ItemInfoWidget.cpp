// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInfoWidget.h"
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

	// 옵션을 한 줄씩 이어 붙인 뒤 한 번에 넣는다. (반복문 안에서 SetText하면 마지막 옵션만 남는다)
	FString OptionString;
	for (const FItemOption& Option : Info.Options)
	{
		OptionString += FString::Printf(TEXT("%s : %.1f\n"),
			*StaticEnum<EItemOptionType>()->GetNameStringByValue((int64)Option.Type),
			Option.Value);
	}
	OptionString.TrimEndInline();	// 마지막 줄의 \n을 지운다.
	mOptionText->SetText(FText::FromString(OptionString));
}

void UItemInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}
