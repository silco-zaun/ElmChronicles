// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "ConfirmWidget.generated.h"

// 예 / 아니요를 묻는 확인 상자. 무엇을 물을지와 "예"를 누르면 할 일은 여는 쪽이 Open으로 넘긴다.
// WBP_PlayerHUD에 하나만 두고 UPlayerHUDWidget::OpenConfirm으로 연다. 화면 전체를 덮어 열려 있는 동안 뒤쪽 창의 버튼을 막는다.
UCLASS()
class ELMCHRONICLES_API UConfirmWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UConfirmWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_Confirm에 같은 이름의 위젯이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mMessageText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mYesButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mNoButton;

private:
	// "예"를 누르면 할 일. Open 때마다 새로 받는다.
	// 버튼 OnClicked 같은 Dynamic 델리게이트와 달리 UFUNCTION이 아닌 함수나 람다도 연결할 수 있다.
	FSimpleDelegate mOnYes;

protected:
	virtual void NativeOnInitialized() override;

public:
	// HUD가 부른다. 문구와 할 일을 바꾸고 보이게 한다. (일시정지 / 커서는 컨트롤러가 맞춘다)
	void Open(const FText& Message, FSimpleDelegate OnYes);
	// HUD가 부른다. 아무것도 하지 않고 숨긴다.
	void Close();

private:
	UFUNCTION()	void OnYesClicked();
	UFUNCTION()	void OnNoClicked();
};
