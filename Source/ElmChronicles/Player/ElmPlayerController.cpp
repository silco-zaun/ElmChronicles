// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmPlayerController.h"
#include "DialogueComponent.h"
#include "InteractionComponent.h"
#include "QuestComponent.h"
#include "../Input/DefaultInput.h"
#include "../Input/DialogueInput.h"
#include "../UI/Game/PlayerHUDWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

AElmPlayerController::AElmPlayerController()
{
	UE_LOG(LogTestOrder, Log, TEXT("AElmPlayerController::Constructor - %s"), *GetName());

	PrimaryActorTick.bCanEverTick = true;

	// 상호작용(주변 대상 찾기, 안내 문구, F 키)은 컴포넌트가 맡는다.
	mInteraction = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	// 대화(대화창, 대화 중 입력, 다음 줄)도 컴포넌트가 맡는다.
	mDialogue = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));
	// 퀘스트(받기, 처치 세기, 완료와 보상)도 컴포넌트가 맡는다.
	mQuest = CreateDefaultSubobject<UQuestComponent>(TEXT("QuestComponent"));

	static ConstructorHelpers::FClassFinder<UPlayerHUDWidget>
		PlayerHUDWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/Game/WBP_PlayerHUD.WBP_PlayerHUD_C'"));

	if (PlayerHUDWidgetClass.Succeeded())
		mPlayerHUDWidgetClass = PlayerHUDWidgetClass.Class;
}

void AElmPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTestOrder, Log,
		TEXT("AMainPlayerController::BeginPlay - Pawn : %s / PlayerState : %s"),
		IsValid(GetPawn()) ? *GetPawn()->GetName() : TEXT("null"),
		IsValid(PlayerState) ? *PlayerState->GetName() : TEXT("null"));

	// HUD는 이 컨트롤러가 조작하는 로컬 플레이어 화면에만 만든다.
	if (IsLocalController() && mPlayerHUDWidgetClass)
	{
		// 첫 번째 인자 this = Owning Player.
		// GetOwningPlayerPawn / GetOwningPlayerState가 이 값을 사용한다.
		mPlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(this, mPlayerHUDWidgetClass);

		if (mPlayerHUDWidget)
		{
			mPlayerHUDWidget->AddToViewport();

			// 시작할 때는 인벤토리를 닫은 상태 + 게임 전용 입력으로 맞춘다.
			SetInventoryOpen(false);

			// Alt+F4 / 창의 X로 닫을 때 바로 끄지 않고 메뉴의 게임 종료처럼 한 번 묻는다.
			// 뷰포트 클라이언트는 레벨을 옮겨도 남으므로, EndPlay에서 풀고 새 레벨의 컨트롤러가 다시 연결한다.
			if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
				ViewportClient->OnWindowCloseRequested().BindUObject(this, &AElmPlayerController::OnWindowCloseRequested);
		}
	}
}

void AElmPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 다른 곳이 연결해 둔 것까지 풀지 않게, 이 컨트롤러가 연결한 경우만 푼다.
	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();

	if (ViewportClient && ViewportClient->OnWindowCloseRequested().IsBoundToObject(this))
		ViewportClient->OnWindowCloseRequested().Unbind();

	if (mPlayerHUDWidget)
	{
		mPlayerHUDWidget->RemoveFromParent();
		mPlayerHUDWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AElmPlayerController::OnPossess(APawn* aPawn)
{
	UE_LOG(LogTestOrder, Log,
		TEXT("AMainPlayerController::OnPossess(Before Super) - Pawn : %s / Controller PlayerState : %s"),
		IsValid(aPawn) ? *aPawn->GetName() : TEXT("null"),
		IsValid(PlayerState) ? *PlayerState->GetName() : TEXT("null"));

	Super::OnPossess(aPawn);

	UE_LOG(LogTestOrder, Log,
		TEXT("AMainPlayerController::OnPossess(After Super) - Pawn PlayerState : %s"),
		(IsValid(aPawn) && IsValid(aPawn->GetPlayerState())) ? *aPawn->GetPlayerState()->GetName() : TEXT("null"));
}

void AElmPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}

void AElmPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult Hit;

}

void AElmPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	if (!IsValid(EnhancedInputComponent))
		return;

	const UDefaultInput* DefaultInputCDO = GetDefault<UDefaultInput>();
	const UDialogueInput* DialogueInputCDO = GetDefault<UDialogueInput>();

	// Enhanced Input은 인자 없는 함수도 바인딩할 수 있다.
	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Inventory")),
		ETriggerEvent::Started, this, &AElmPlayerController::ToggleInventory);

	// 핸들러는 컨트롤러가 아니라 컴포넌트의 함수다. TObjectPtr는 템플릿이 타입을 읽지 못하므로 .Get()으로 넘긴다.
	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Interact")),
		ETriggerEvent::Started, mInteraction.Get(), &UInteractionComponent::InteractKey);

	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Menu")),
		ETriggerEvent::Started, this, &AElmPlayerController::ToggleMenu);

	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Quest")),
		ETriggerEvent::Started, this, &AElmPlayerController::ToggleQuest);

	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("CharacterInfo")),
		ETriggerEvent::Started, this, &AElmPlayerController::ToggleCharacterInfo);

	// IMC_Dialogue에만 매핑되어 있으므로 대화 중에만 들어온다. 대화용 액션은 UDialogueInput이 들고 있다.
	EnhancedInputComponent->BindAction(DialogueInputCDO->FindInputAction(TEXT("DialogueNext")),
		ETriggerEvent::Started, mDialogue.Get(), &UDialogueComponent::AdvanceDialogue);
}

void AElmPlayerController::ToggleInventory()
{
	if (mPlayerHUDWidget)
		SetInventoryOpen(!mPlayerHUDWidget->IsInventoryVisible());
}

void AElmPlayerController::SetInventoryOpen(bool Open)
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->SetInventoryVisible(Open);
	RefreshInputMode();
}

void AElmPlayerController::ToggleQuest()
{
	if (mPlayerHUDWidget)
		SetQuestOpen(!mPlayerHUDWidget->IsQuestVisible());
}

void AElmPlayerController::SetQuestOpen(bool Open)
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->SetQuestVisible(Open);
	RefreshInputMode();
}

void AElmPlayerController::ToggleCharacterInfo()
{
	if (mPlayerHUDWidget)
		SetCharacterInfoOpen(!mPlayerHUDWidget->IsCharacterInfoVisible());
}

void AElmPlayerController::SetCharacterInfoOpen(bool Open)
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->SetCharacterInfoVisible(Open);
	RefreshInputMode();
}

void AElmPlayerController::ToggleMenu()
{
	if (!mPlayerHUDWidget)
		return;

	SetMenuOpen(!mPlayerHUDWidget->IsMenuVisible());
}

void AElmPlayerController::SetMenuOpen(bool Open)
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->SetMenuVisible(Open);
	RefreshInputMode();
}

void AElmPlayerController::OpenConfirm(const FText& Message, FSimpleDelegate OnYes)
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->OpenConfirm(Message, MoveTemp(OnYes));
	RefreshInputMode();
}

void AElmPlayerController::CloseConfirm()
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->CloseConfirm();
	RefreshInputMode();
}

void AElmPlayerController::RequestQuit()
{
	OpenConfirm(NSLOCTEXT("ElmMenu", "QuitConfirm", "게임을 종료할까요?"),
		FSimpleDelegate::CreateUObject(this, &AElmPlayerController::OnQuitConfirmed));
}

bool AElmPlayerController::OnWindowCloseRequested()
{
	// HUD가 없으면 물을 곳이 없으니 그냥 닫는다. (막기만 하면 아무것도 뜨지 않은 채 창이 닫히지 않는다)
	if (!mPlayerHUDWidget)
		return true;

	// false = 창을 닫지 않는다. 대신 확인 상자를 띄우고, "예"를 누르면 OnQuitConfirmed가 끈다.
	RequestQuit();
	return false;
}

void AElmPlayerController::OnQuitConfirmed()
{
	// 내부적으로 "quit" 콘솔 명령을 보낸다. 패키지 게임은 프로그램이 닫히고, PIE는 플레이만 끝난다(에디터는 그대로).
	// 창 닫기 요청을 거치지 않고 끝나므로 OnWindowCloseRequested가 다시 불리지 않는다.
	// 마지막 인자 false: 플랫폼 제약을 무시하지 않는다("quit force"를 쓰지 않는다). PC에서는 차이가 없다.
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AElmPlayerController::RefreshInputMode()
{
	const bool UIOpen = mPlayerHUDWidget && mPlayerHUDWidget->IsAnyWindowOpen();

	if (UIOpen)
	{
		// 이동 같은 게임 입력은 유지하면서 마우스로 UI를 조작한다.
		// 카메라 회전은 UI 바깥을 클릭한 채로 드래그할 때만 동작한다.
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}

	SetShowMouseCursor(UIOpen);
}
