// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueWidget.h"

UDialogueWidget::UDialogueWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UDialogueWidget::SetLine(const FString& Speaker, const FString& Text)
{
	// 테이블 글자는 FString이라 TextBlock에 넣을 때 FText로 바꾼다.
	mSpeakerText->SetText(FText::FromString(Speaker));
	mDialogueText->SetText(FText::FromString(Text));
}
