// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "../../Player/ElmPlayerController.h"

UMenuWidget::UMenuWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다. (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mResumeButton->OnClicked.AddDynamic(this, &UMenuWidget::OnResumeClicked);
}

void UMenuWidget::Refresh()
{
	// 지난번에 띄운 문구와 열어 둔 확인 상자는 지운다.
	SetMessage(FText::GetEmpty());
	ShowQuitConfirm(false);
}

void UMenuWidget::OnResumeClicked()
{
	// 닫으면서 일시정지도 풀어야 하므로 B 키와 같은 길(컨트롤러)로 닫는다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->SetMenuOpen(false);
}

void UMenuWidget::SetMessage(const FText& Message)
{
	mMessageText->SetText(Message);
	// 글자일 뿐이므로 클릭을 가로채지 않게 한다. 비어 있으면 자리도 차지하지 않게 접는다.
	mMessageText->SetVisibility(Message.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UMenuWidget::ShowQuitConfirm(bool Show)
{
	// 둘 중 하나만 보인다. 상자 자체는 클릭을 받을 필요가 없고 안의 버튼만 받으면 되므로 SelfHitTestInvisible.
	mMainBox->SetVisibility(Show ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	mQuitConfirmBox->SetVisibility(Show ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
