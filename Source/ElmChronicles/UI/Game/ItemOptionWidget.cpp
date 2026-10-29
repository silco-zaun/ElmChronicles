// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemOptionWidget.h"
#include "ItemOptionEntryObject.h"


UItemOptionWidget::UItemOptionWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UItemOptionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UItemOptionWidget::SetData(UObject* EntryObject)
{
	UItemOptionEntryObject* Obj = Cast<UItemOptionEntryObject>(EntryObject);

	if (!Obj)
	{
		UE_LOG(LogTestDebug, Warning, TEXT("ItemOptionEntryObject is null."));

		return;
	}


	mOptionNameText->SetText(FText::FromString(Obj->GetOptionName()));

	FString Value = FString::Printf(TEXT("%.2f"), Obj->GetOption());

	mOptionValueText->SetText(FText::FromString(Value));

	UE_LOG(LogTestDebug, Warning, TEXT("Name : %s, Option : %s"), *Obj->GetOptionName(), *Value);
}
