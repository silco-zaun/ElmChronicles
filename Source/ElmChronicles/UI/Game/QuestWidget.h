// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "QuestWidget.generated.h"

// P 키로 여는 퀘스트 창. 받은 퀘스트(진행 중 + 완료)의 제목, 설명, 목표, 보상을 보여 준다.
// 추적창처럼 퀘스트 컴포넌트의 변경 알림을 구독해서 스스로 다시 그린다. 열고 닫기는 컨트롤러(SetQuestOpen)가 한다.
UCLASS()
class ELMCHRONICLES_API UQuestWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UQuestWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_Quest에 같은 이름의 위젯이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mCloseButton;

	// 퀘스트 목록 글자. 길어지면 스크롤되도록 WBP에서 ScrollBox 안에 둔다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mQuestListText;

	// 구독한 퀘스트 컴포넌트. 위젯이 사라질 때 구독을 끊으려고 들고 있는다.
	TWeakObjectPtr<class UQuestComponent> mQuest;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 버튼의 OnClicked는 Dynamic 델리게이트라서 UFUNCTION 함수만 연결할 수 있다.
	UFUNCTION()	void OnCloseClicked();
	void Refresh();
};
