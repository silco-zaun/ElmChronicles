// Fill out your copyright notice in the Description page of Project Settings.


#include "ElmPlayerController.h"
#include "../Characters/PlayerCharacter.h"
#include "../ElmCollisionChannels.h"
#include "../Input/DefaultInput.h"
#include "../Interaction/Interactable.h"
#include "../UI/Game/PlayerHUDWidget.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

AElmPlayerController::AElmPlayerController()
{
	UE_LOG(LogTestOrder, Log, TEXT("AElmPlayerController::Constructor - %s"), *GetName());

	PrimaryActorTick.bCanEverTick = true;

	// 마우스 커서를 보이게 한다.
	//bShowMouseCursor = true;

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

	// 상호작용 대상 검색. 안내 문구를 띄울 화면이 있는 로컬 플레이어만 한다.
	if (IsLocalController())
	{
		GetWorldTimerManager().SetTimer(mInteractTimer, this,
			&AElmPlayerController::UpdateInteractTarget, InteractScanInterval, true);
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

	// Enhanced Input은 인자 없는 함수도 바인딩할 수 있다.
	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Inventory")),
		ETriggerEvent::Started, this, &AElmPlayerController::ToggleInventory);

	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Interact")),
		ETriggerEvent::Started, this, &AElmPlayerController::InteractKey);

	EnhancedInputComponent->BindAction(DefaultInputCDO->FindInputAction(TEXT("Menu")),
		ETriggerEvent::Started, this, &AElmPlayerController::ToggleMenu);
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

void AElmPlayerController::UpdateInteractTarget()
{
	// 지역 변수 이름을 Player로 지으면 APlayerController::Player(UPlayer*) 멤버를 가려서 C4458 오류가 난다.
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	AActor* NewTarget = nullptr;

	if (IsValid(PlayerCharacter) && !PlayerCharacter->IsDead())
	{
		// Interaction 채널에 응답하는 컴포넌트(NPC 캡슐)만 걸린다. 벽, 몬스터, 플레이어는 Ignore라 걸리지 않는다.
		// (Lyra의 GrantNearbyInteraction도 플레이어 주변 구를 타이머로 Overlap해서 찾는다)
		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Interact), false, PlayerCharacter);

		GetWorld()->OverlapMultiByChannel(Overlaps, PlayerCharacter->GetActorLocation(), FQuat::Identity,
			ElmChannels::Interaction, FCollisionShape::MakeSphere(InteractRadius), Params);

		double NearestDistSq = TNumericLimits<double>::Max();

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Actor = Overlap.GetActor();
			// 인터페이스를 구현한 액터만 Cast가 성공한다. (NPC인지 상자인지는 몰라도 된다)
			IInteractable* Interactable = Cast<IInteractable>(Actor);

			if (!Interactable || !Interactable->CanInteract(PlayerCharacter))
				continue;

			// 여러 명이 걸리면 가장 가까운 대상
			const double DistSq = FVector::DistSquared(PlayerCharacter->GetActorLocation(), Actor->GetActorLocation());

			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NewTarget = Actor;
			}
		}
	}

	mInteractTarget = NewTarget;

	// 대상이 사라지거나 문구가 바뀌어도 바로 반영되도록 매번 갱신한다. (0.1초마다라 부담이 없다)
	if (mPlayerHUDWidget)
	{
		IInteractable* Interactable = Cast<IInteractable>(NewTarget);
		mPlayerHUDWidget->SetInteractPrompt(Interactable ? Interactable->GetInteractText() : FText::GetEmpty());
	}
}

void AElmPlayerController::InteractKey()
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	IInteractable* Interactable = Cast<IInteractable>(mInteractTarget.Get());

	// 죽은 뒤 첫 검색(0.1초) 전에는 mInteractTarget이 남아 있다.
	// 죽은 채로 포탈을 타면 HP 0이 메모에 적힌다.
	if (!IsValid(PlayerCharacter) || PlayerCharacter->IsDead() || !Interactable ||
		!Interactable->CanInteract(PlayerCharacter))
		return;

	Interactable->Interact(PlayerCharacter);
}
