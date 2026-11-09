// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

// 메인 화면의 컨트롤러. 메뉴 위젯(WBP_MainMenu)을 화면 전체에 띄우고, 마우스로 UI만 조작하게 한다.
// Stack O Bot은 같은 일을 HUD 클래스(HUD_MainMenu)가 하지만, 이 프로젝트는 게임 HUD도 컨트롤러가 만들므로(AElmPlayerController) 맞춘다.
UCLASS()
class ELMCHRONICLES_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainMenuPlayerController();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMainMenuWidget> mMainMenuWidgetClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UMainMenuWidget> mMainMenuWidget;

protected:
	virtual void BeginPlay() override;
};
