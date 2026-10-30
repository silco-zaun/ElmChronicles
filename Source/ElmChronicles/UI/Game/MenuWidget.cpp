// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "../../Characters/PlayerCharacter.h"
#include "../../GameModes/ElmGameModeBase.h"
#include "../../Player/ElmPlayerController.h"
#include "../../Subsystem/SaveSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

UMenuWidget::UMenuWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯이 처음 만들어질 때 한 번만 연결한다. (NativeConstruct는 화면에 붙을 때마다 불려서 두 번 연결될 수 있다)
	mResumeButton->OnClicked.AddDynamic(this, &UMenuWidget::OnResumeClicked);
	mRecallButton->OnClicked.AddDynamic(this, &UMenuWidget::OnRecallClicked);
	mSaveButton->OnClicked.AddDynamic(this, &UMenuWidget::OnSaveClicked);
	mQuitButton->OnClicked.AddDynamic(this, &UMenuWidget::OnQuitClicked);
	mQuitYesButton->OnClicked.AddDynamic(this, &UMenuWidget::OnQuitYesClicked);
	mQuitNoButton->OnClicked.AddDynamic(this, &UMenuWidget::OnQuitNoClicked);
}

void UMenuWidget::Refresh()
{
	// 지난번에 띄운 문구와 열어 둔 확인 상자는 지운다.
	SetMessage(FText::GetEmpty());
	ShowQuitConfirm(false);

	// 메뉴가 열려 있는 동안은 게임이 멈춰 있으므로, 열 때 한 번 정하면 닫을 때까지 맞다.
	const APlayerCharacter* Player = GetOwningPlayerPawn<APlayerCharacter>();
	const AElmGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AElmGameModeBase>();

	bool IsInCombat = GameMode && IsValid(Player) && !Player->IsDead() && !GameMode->IsInCombat(Player);
	mRecallButton->SetIsEnabled(IsInCombat);
	mSaveButton->SetIsEnabled(IsInCombat);

	// 죽어 있을 때는 버튼만 끈다(이유가 화면에 보인다). 전투 중일 때만 왜 안 되는지 알려 준다.
	if (GameMode && IsValid(Player) && !Player->IsDead() && GameMode->IsInCombat(Player))
		SetMessage(NSLOCTEXT("ElmMenu", "InCombat", "전투 중에는 할 수 없습니다."));
}

void UMenuWidget::OnResumeClicked()
{
	// 닫으면서 일시정지도 풀어야 하므로 B 키와 같은 길(컨트롤러)로 닫는다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->SetMenuOpen(false);
}

void UMenuWidget::OnRecallClicked()
{
	AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>();
	AElmGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AElmGameModeBase>();

	if (!IsValid(PlayerController) || !GameMode)
		return;

	// 일시정지 중에는 월드 타이머가 돌지 않아서 TravelToLevel의 페이드 → OpenLevel이 영영 불리지 않는다.
	// 그래서 메뉴를 닫아 일시정지를 먼저 풀고 귀환을 부탁한다.
	PlayerController->SetMenuOpen(false);
	GameMode->Recall(PlayerController->GetPawn<APlayerCharacter>());
}

void UMenuWidget::OnSaveClicked()
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	APlayerCharacter* Player = GetOwningPlayerPawn<APlayerCharacter>();

	// 저장은 타이머를 쓰지 않아서 일시정지 중에도 바로 끝난다. 메뉴는 열어 둔 채 결과만 알려 준다.
	const bool Saved = SaveSubsystem && SaveSubsystem->SaveToDisk(Player);

	SetMessage(Saved ? NSLOCTEXT("ElmMenu", "Saved", "저장했습니다.")
		: NSLOCTEXT("ElmMenu", "SaveFailed", "저장하지 못했습니다."));
}

void UMenuWidget::OnQuitClicked()
{
	// 잘못 눌러도 바로 꺼지지 않게 한 번 더 묻는다. 버튼 목록 자리에 확인 상자를 띄운다.
	ShowQuitConfirm(true);
}

void UMenuWidget::OnQuitYesClicked()
{
	// 내부적으로 "quit" 콘솔 명령을 보낸다. 패키지 게임은 프로그램이 닫히고, PIE는 플레이만 끝난다(에디터는 그대로).
	// 마지막 인자 false: 플랫폼 제약을 무시하지 않는다("quit force"를 쓰지 않는다). PC에서는 차이가 없다.
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UMenuWidget::OnQuitNoClicked()
{
	ShowQuitConfirm(false);
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
