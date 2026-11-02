// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestWidget.h"
#include "../../Data/Tables/QuestTableRow.h"
#include "../../Player/ElmPlayerController.h"
#include "../../Player/QuestComponent.h"

UQuestWidget::UQuestWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UQuestWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다. (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mCloseButton->OnClicked.AddDynamic(this, &UQuestWidget::OnCloseClicked);
}

void UQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 추적창과 같은 방식으로 퀘스트 컴포넌트를 구독한다. 창이 닫혀 있어도 미리 그려 두므로 열 때 따로 할 일이 없다.
	const AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>();
	UQuestComponent* Quest = IsValid(PlayerController) ? PlayerController->GetQuest() : nullptr;

	if (!Quest)
		return;

	mQuest = Quest;
	Quest->AddQuestChangeCallback(this, &UQuestWidget::Refresh);

	// 구독하기 전에 이미 있던 목록(레벨 이동 뒤 메모에서 되살린 퀘스트)으로 한 번 그린다.
	Refresh();
}

void UQuestWidget::NativeDestruct()
{
	if (UQuestComponent* Quest = mQuest.Get())
		Quest->RemoveCallbacks(this);

	mQuest.Reset();

	Super::NativeDestruct();
}

void UQuestWidget::OnCloseClicked()
{
	// 닫으면서 커서와 입력 모드도 되돌려야 하므로 P 키와 같은 길(컨트롤러)로 닫는다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->SetQuestOpen(false);
}

void UQuestWidget::Refresh()
{
	const UQuestComponent* Quest = mQuest.Get();

	if (!Quest)
		return;

	// 받은 순서대로 한 덩어리씩 만든다. 완료한 퀘스트도 [완료]로 남겨 둔다.
	TArray<FText> Entries;

	for (const FQuestProgress& Progress : Quest->GetQuests())
	{
		const FQuestTableRow* Info = Quest->FindQuestInfo(Progress.QuestRow);

		if (!Info)
			continue;

		const FText StateText = Progress.State == EQuestState::Completed ?
			NSLOCTEXT("ElmQuest", "StateCompleted", "완료") : NSLOCTEXT("ElmQuest", "StateInProgress", "진행 중");

		Entries.Add(FText::Format(
			NSLOCTEXT("ElmQuest", "QuestEntry", "[{0}] {1}\n{2}\n목표 : {3}\n보상 : 경험치 {4}, 골드 {5}"),
			StateText, FText::FromString(Info->Title), FText::FromString(Info->Description),
			Quest->MakeObjectiveText(Progress), Info->RewardExp, Info->RewardGold));
	}

	if (Entries.IsEmpty())
		mQuestListText->SetText(NSLOCTEXT("ElmQuest", "NoQuest", "받은 퀘스트가 없습니다."));
	else
		mQuestListText->SetText(FText::Join(FText::FromString(TEXT("\n\n")), Entries));
}
