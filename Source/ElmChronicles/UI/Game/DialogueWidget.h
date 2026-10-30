// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "DialogueWidget.generated.h"

// 화면 아래 대화창. 말하는 사람과 대사 한 줄을 보여 준다.
// 어떤 줄을 보여 줄지는 UDialogueComponent가 정하고, 이 위젯은 받은 글자를 그리기만 한다.
UCLASS()
class ELMCHRONICLES_API UDialogueWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UDialogueWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_Dialogue에 같은 이름의 TextBlock이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mSpeakerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mDialogueText;

public:
	void SetLine(const FString& Speaker, const FString& Text);
};
