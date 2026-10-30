// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmPlayerController.h"
#include "DialogueComponent.h"
#include "InteractionComponent.h"
#include "../Input/DefaultInput.h"
#include "../Input/DialogueInput.h"
#include "../UI/Game/PlayerHUDWidget.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "UObject/ConstructorHelpers.h"

AElmPlayerController::AElmPlayerController()
{
	UE_LOG(LogTestOrder, Log, TEXT("AElmPlayerController::Constructor - %s"), *GetName());

	PrimaryActorTick.bCanEverTick = true;

	// 상호작용(주변 대상 찾기, 안내 문구, F 키)은 컴포넌트가 맡는다.
	mInteraction = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	// 대화(대화창, 대화 중 입력, 다음 줄)도 컴포넌트가 맡는다.
	mDialogue = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

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
		}
	}
}

void AElmPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

void AElmPlayerController::ToggleMenu()
{
	if (mPlayerHUDWidget)
		SetMenuOpen(!mPlayerHUDWidget->IsMenuVisible());
}

void AElmPlayerController::SetMenuOpen(bool Open)
{
	if (!mPlayerHUDWidget)
		return;

	mPlayerHUDWidget->SetMenuVisible(Open);

	// 멈추면 월드의 Tick과 타이머가 모두 선다. (몬스터, 애니메이션, 사망 후 병원 이동 타이머까지)
	// 입력은 "Trigger when Paused"를 켠 액션(IA_Menu)만 들어온다. 그래서 B로 다시 닫을 수 있고 이동 / 공격 / I / E는 막힌다.
	SetPause(Open);
	RefreshInputMode();
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
