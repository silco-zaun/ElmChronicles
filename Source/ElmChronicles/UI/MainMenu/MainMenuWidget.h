// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "MainMenuWidget.generated.h"

// 메인 화면 메뉴. 화면 전체를 덮는 위젯이고, 뒤로 MainMenu 맵(MenuCamera)이 보인다.
// 버튼은 일을 맡은 곳(AMainMenuGameMode)에 부탁만 한다. (게임 안 메뉴 UMenuWidget과 같은 방식)
UCLASS()
class ELMCHRONICLES_API UMainMenuWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_MainMenu에 같은 이름의 위젯이 있어야 한다.
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mNewGameButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mContinueButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mOptionsButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mQuitButton;

protected:
	virtual void NativeOnInitialized() override;

private:
	// 버튼의 OnClicked는 Dynamic 델리게이트라서 UFUNCTION 함수만 연결할 수 있다.
	UFUNCTION()	void OnNewGameClicked();
	UFUNCTION()	void OnContinueClicked();
	UFUNCTION()	void OnOptionsClicked();
	UFUNCTION()	void OnQuitClicked();

	// 맵을 떠나기 시작하면 부른다. 버튼을 더 누르지 못하게 하고 메뉴를 흐리게 지운다.
	void HideForTravel();
};
