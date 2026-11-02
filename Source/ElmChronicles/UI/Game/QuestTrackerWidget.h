// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "QuestTrackerWidget.generated.h"

// HUD 오른쪽의 퀘스트 추적창. 진행 중인 퀘스트의 제목과 목표("해골전사 처치 0 / 1")를 보여 준다.
// 퀘스트 컴포넌트의 변경 알림을 구독해서 스스로 다시 그린다. (상태바가 ASC를 구독하는 것과 같은 방식)
// 진행 중인 퀘스트가 없으면 숨는다.
UCLASS()
class ELMCHRONICLES_API UQuestTrackerWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UQuestTrackerWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_QuestTracker에 같은 이름의 TextBlock이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mTrackerText;

	// 구독한 퀘스트 컴포넌트. 위젯이 사라질 때 구독을 끊으려고 들고 있는다.
	TWeakObjectPtr<class UQuestComponent> mQuest;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void Refresh();
};
