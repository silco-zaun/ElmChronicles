// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestTrackerWidget.h"
#include "../../Data/Tables/QuestTableRow.h"
#include "../../Player/ElmPlayerController.h"
#include "../../Player/QuestComponent.h"

UQuestTrackerWidget::UQuestTrackerWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UQuestTrackerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 퀘스트는 플레이어 컨트롤러의 퀘스트 컴포넌트가 들고 있다.
	// HUD는 컨트롤러 BeginPlay에서 만들어지므로 이때는 컴포넌트가 이미 있다.
	const AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>();
	UQuestComponent* Quest = IsValid(PlayerController) ? PlayerController->GetQuest() : nullptr;

	if (!Quest)
		return;

	mQuest = Quest;
	Quest->AddQuestChangeCallback(this, &UQuestTrackerWidget::Refresh);

	// 구독하기 전에 이미 있던 목록(레벨 이동 뒤 메모에서 되살린 퀘스트)으로 한 번 그린다.
	Refresh();
}

void UQuestTrackerWidget::NativeDestruct()
{
	if (UQuestComponent* Quest = mQuest.Get())
		Quest->RemoveCallbacks(this);

	mQuest.Reset();

	Super::NativeDestruct();
}

void UQuestTrackerWidget::Refresh()
{
	const UQuestComponent* Quest = mQuest.Get();

	if (!Quest)
		return;

	// 진행 중인 퀘스트마다 "제목 + 줄바꿈 + - 목표" 한 덩어리를 만든다. 완료한 퀘스트는 퀘스트 창(P)에서만 보인다.
	TArray<FText> Entries;

	for (const FQuestProgress& Progress : Quest->GetQuests())
	{
		if (Progress.State != EQuestState::InProgress)
			continue;

		const FQuestTableRow* Info = Quest->FindQuestInfo(Progress.QuestRow);

		if (!Info)
			continue;

		Entries.Add(FText::Format(NSLOCTEXT("ElmQuest", "TrackerEntry", "{0}\n- {1}"),
			FText::FromString(Info->Title), Quest->MakeObjectiveText(Progress)));
	}

	// 퀘스트가 여러 개면 사이를 한 줄 띄운다.
	mTrackerText->SetText(FText::Join(FText::FromString(TEXT("\n\n")), Entries));

	// 진행 중인 퀘스트가 없으면 통째로 숨는다. 보일 때도 글자일 뿐이므로 클릭을 가로채지 않게 한다.
	SetVisibility(Entries.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}
