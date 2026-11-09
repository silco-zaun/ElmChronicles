// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerController.h"
#include "../UI/MainMenu/MainMenuWidget.h"
#include "UObject/ConstructorHelpers.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	static ConstructorHelpers::FClassFinder<UMainMenuWidget>
		MainMenuWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/MainMenu/WBP_MainMenu.WBP_MainMenu_C'"));

	if (MainMenuWidgetClass.Succeeded())
		mMainMenuWidgetClass = MainMenuWidgetClass.Class;
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || !mMainMenuWidgetClass)
		return;

	mMainMenuWidget = CreateWidget<UMainMenuWidget>(this, mMainMenuWidgetClass);

	if (!mMainMenuWidget)
		return;

	mMainMenuWidget->AddToViewport();

	// 메인 화면에는 게임 입력이 없다. 마우스는 UI만 누르고, 창 밖으로 자유롭게 나갈 수 있다.
	// 키보드 포커스를 줄 위젯은 정하지 않는다. 버튼이 포커스를 받지 않게 해 두어서(Is Focusable 끔) 마우스로만 고른다.
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}
