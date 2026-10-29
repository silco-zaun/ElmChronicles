// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldInfoWidget.h"

UWorldInfoWidget::UWorldInfoWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UWorldInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UWorldInfoWidget::SetInfoName(const FString& Name)
{
	mNameText->SetText(FText::FromString(Name));
}

void UWorldInfoWidget::SetHP(float HP, float HPMax)
{
	mHPBar->SetPercent(HP / HPMax);
}
