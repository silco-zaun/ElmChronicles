// Fill out your copyright notice in the Description page of Project Settings.


#include "ConfirmWidget.h"
#include "../../Player/ElmPlayerController.h"

UConfirmWidget::UConfirmWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UConfirmWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다. (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mYesButton->OnClicked.AddDynamic(this, &UConfirmWidget::OnYesClicked);
	mNoButton->OnClicked.AddDynamic(this, &UConfirmWidget::OnNoClicked);

	// 닫힌 채로 시작한다.
	Close();
}

void UConfirmWidget::Open(const FText& Message, FSimpleDelegate OnYes)
{
	mMessageText->SetText(Message);
	mOnYes = MoveTemp(OnYes);

	// 반투명 배경까지 클릭을 받게 Visible로 둔다. 그래야 덮고 있는 뒤쪽 버튼이 눌리지 않는다.
	SetVisibility(ESlateVisibility::Visible);
}

void UConfirmWidget::Close()
{
	// 닫힌 뒤에는 할 일이 실행될 일이 없으므로 비운다. (람다가 잡아 둔 값도 같이 놓는다)
	mOnYes.Unbind();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UConfirmWidget::OnYesClicked()
{
	// 할 일을 꺼내 두고 먼저 닫는다. 할 일 안에서 다시 OpenConfirm(다음 질문)을 불러도 곧바로 닫혀 버리지 않는다.
	FSimpleDelegate OnYes = MoveTemp(mOnYes);

	// 닫으면서 일시정지와 커서도 되돌려야 하므로 열 때와 같은 길(컨트롤러)로 닫는다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->CloseConfirm();

	// CreateUObject로 연결했으면 할 일을 맡긴 쪽이 그사이 사라졌을 때 연결이 풀려 있어 아무 일도 하지 않는다.
	OnYes.ExecuteIfBound();
}

void UConfirmWidget::OnNoClicked()
{
	// "예"와 같은 이유로 컨트롤러를 거쳐 닫는다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->CloseConfirm();
}
