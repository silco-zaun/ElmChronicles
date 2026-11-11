// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "../../GameModes/MainMenuGameMode.h"
#include "../../Player/PopupComponent.h"
#include "../../Subsystem/SaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다.
	// (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mNewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameClicked);
	mContinueButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinueClicked);
	mOptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
	mQuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);

	// 세이브 파일이 없으면 이어 갈 게임이 없으므로 끈다.
	// 메인 화면에서는 저장할 수 없으므로(캐릭터가 없다) 처음 만들 때 한 번 정하면 화면을 떠날 때까지 맞다.
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	mContinueButton->SetIsEnabled(SaveSubsystem && SaveSubsystem->HasSaveOnDisk());
}

void UMainMenuWidget::OnNewGameClicked()
{
	AMainMenuGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainMenuGameMode>();

	if (GameMode && GameMode->StartNewGame())
		HideForTravel();
}

void UMainMenuWidget::OnContinueClicked()
{
	AMainMenuGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainMenuGameMode>();

	if (GameMode && GameMode->ContinueGame())
	{
		HideForTravel();
		return;
	}

	// 파일은 있는데 깨졌거나 읽지 못한 경우다. 다시 눌러도 같으므로 끈다.
	mContinueButton->SetIsEnabled(false);
}

void UMainMenuWidget::OnOptionsClicked()
{
	// 옵션 창은 이 위젯이 아니라 팝업 층에 있다.
	// 게임 안 메뉴(B)도 같은 길로 연다(5단계).
	if (UPopupComponent* Popup = UPopupComponent::Get(GetOwningPlayer()))
		Popup->OpenOptions();
}

void UMainMenuWidget::OnQuitClicked()
{
	// 메인 화면에는 잃을 진행이 없으므로 묻지 않고 바로 끈다. (게임 안에서는 AElmPlayerController::RequestQuit가 묻는다)
	// 패키지 게임은 프로그램이 닫히고, PIE는 플레이만 끝난다.
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UMainMenuWidget::HideForTravel()
{
	// 카메라 페이드는 3D 화면만 어둡게 하고 UMG는 덮지 않는다. 그대로 두면 검은 화면에 버튼만 떠 있다.
	// 클릭은 바로 막고(HitTestInvisible), 모습은 WBP의 FadeOut 애니메이션(0.5초 = 페이드 시간)으로 지운다.
	// FadeOut이 없으면 PlayWidgetAnimation이 아무것도 하지 않는다. (버튼이 보인 채 어두워질 뿐 동작은 같다)
	SetVisibility(ESlateVisibility::HitTestInvisible);
	PlayWidgetAnimation(TEXT("FadeOut"));
}
