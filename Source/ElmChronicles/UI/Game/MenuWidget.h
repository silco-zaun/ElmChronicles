// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "MenuWidget.generated.h"

// B 키로 여는 메뉴 창. 열려 있는 동안 게임은 멈춰 있다. (AElmPlayerController::SetMenuOpen)
// 버튼이 할 일은 그 일을 맡은 곳(컨트롤러, GameMode, SaveSubsystem)에 부탁하고, 이 위젯은 결과 문구만 띄운다.
UCLASS()
class ELMCHRONICLES_API UMenuWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_Menu에 같은 이름의 위젯이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mRecallButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mSaveButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mQuitButton;

	// "저장했습니다", "전투 중에는 귀환할 수 없습니다" 같은 안내. 비어 있으면 숨긴다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mMessageText;

protected:
	virtual void NativeOnInitialized() override;

public:
	// 메뉴를 열 때마다 HUD가 부른다. 지난번 문구를 지우고, 지금 상황에 맞게 버튼을 켜고 끈다.
	void Refresh();

private:
	// 버튼의 OnClicked는 Dynamic 델리게이트라서 UFUNCTION 함수만 연결할 수 있다.
	UFUNCTION()	void OnResumeClicked();
	UFUNCTION()	void OnRecallClicked();
	UFUNCTION()	void OnSaveClicked();
	UFUNCTION()	void OnQuitClicked();

	void SetMessage(const FText& Message);
};
