// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "SlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "../../Player/ElmPlayerController.h"
#include "../../Player/InventoryComponent.h"
#include "GameFramework/Pawn.h"

UInventoryWidget::UInventoryWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<USlotWidget>
		SlotWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/Game/WBP_Slot.WBP_Slot_C'"));

	if (SlotWidgetClass.Succeeded())
		mSlotWidgetClass = SlotWidgetClass.Class;
}

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯 블루프린트에서 변수여부 체크가 안되어 있을 경우 GetWidgetFromName 함수를
	// 이용하여 배치된 위젯이 객체를 얻어올 수 있다.
	mTitleBar = GetWidgetFromName(TEXT("TitleBar"));	// 어떤 종류의 위젯이든 받을 수 있다
	if (!mTitleBar)
		UE_LOG(LogTestDebug, Error, TEXT("InventoryWidget : TitleBar 위젯을 찾지 못했습니다."));

	mCloseButton->OnClicked.AddDynamic(this, &UInventoryWidget::CloseInventory);
}

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 이 위젯을 소유한 플레이어의 Pawn에서 인벤토리 컴포넌트를 직접 가져온다.
	APawn* Pawn = GetOwningPlayerPawn();
	UInventoryComponent* Inventory = Pawn ? Pawn->FindComponentByClass<UInventoryComponent>() : nullptr;

	if (Inventory)
		InitInventory(Inventory);
}

void UInventoryWidget::NativeDestruct()
{
	// 화면에서 떨어질 때 구독을 해제한다. 다시 붙으면 NativeConstruct에서 다시 구독한다.
	if (UInventoryComponent* Inventory = mInventoryComponent.Get())
		Inventory->RemoveCallbacks(this);

	Super::NativeDestruct();
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// 마우스 왼쪽버튼일 경우
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 마우스 위치가 TitleBar 안일 경우 동작한다.
		// GetScreenSpacePosition : 화면에서의 마우스 위치를 얻어온다.
		if (mTitleBar && mTitleBar->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			mIsDragging = true;
			mDragStartMousePos = InMouseEvent.GetScreenSpacePosition();
			// GetRenderTranform : 출력할 Transform 정보.
			mDragStartWidgetPos = GetRenderTransform().Translation;

			// Handled()로 처리를 하여 이 위젯이 마우스 클릭 이벤트를 처리했다는 의미가 된다.
			// CaptureMouse 함수로 TakeWidget을 지정하면 마우스를 누른 뒤부터는 마우스 커서가
			// 위젯 밖으로 나가도 이 위젯이 계속 마우스 이벤트를 받게 된다.
			return FReply::Handled().CaptureMouse(TakeWidget());
		}
	}

	return FReply::Unhandled();
}

FReply UInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// 마우스 왼쪽버튼일 경우
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		mIsDragging = false;

		// Mouse Capture를 해제한다.
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FReply UInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (mIsDragging)
	{
		// 현재 마우스 위치를 얻어온다.
		FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();

		// Drag를 시작한 위치와 비교하여 얼마나 떨어졌는지 구한다.
		FVector2D MouseDelta = CurrentMousePos - mDragStartMousePos;

		// DPI Scale을 얻어온다.
		float ViewScale = UWidgetLayoutLibrary::GetViewportScale(this);

		// 마우스가 이동한 양을 ViewScale을 이용하여 화면에서 움직인 양으로 변경한다.
		FVector2D WidgetPos = mDragStartWidgetPos + MouseDelta / ViewScale;

		SetRenderTranslation(WidgetPos);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UInventoryWidget::InitInventory(UInventoryComponent* InventoryComponent)
{
	mInventoryComponent = InventoryComponent;

	// 다시 호출되어도 슬롯이 중복으로 쌓이지 않도록 기존 슬롯을 모두 제거한다.
	mSlotGrid->ClearChildren();
	mSlotWidgetArray.Reset();

	if (!mSlotWidgetClass)
	{
		UE_LOG(LogTestDebug, Error, TEXT("InitInventory : mSlotWidgetClass가 지정되지 않았습니다."));
		return;
	}

	int32 MaxCount = InventoryComponent->GetInventoryMaxCount();

	for (int32 i = 0; i < MaxCount; ++i)
	{
		// 슬롯 위젯을 생성한다. 이 시점에 슬롯의 NativeOnInitialized까지 호출된다.
		USlotWidget* SlotWidget = CreateWidget<USlotWidget>(this, mSlotWidgetClass);

		// 슬롯이 자신이 몇 번 칸인지, 어느 인벤토리 소속인지 알 수 있게 한다.
		SlotWidget->SetSlotIndex(i);
		SlotWidget->SetSlotType(ESlotType::Inventory);
		SlotWidget->SetInventoryComponent(InventoryComponent);

		// i번째 슬롯을 (행, 열) 위치에 붙인다.
		UUniformGridSlot* GridSlot = mSlotGrid->AddChildToUniformGrid(SlotWidget, i / mColumnCount, i % mColumnCount);
		GridSlot->SetHorizontalAlignment(HAlign_Fill);
		GridSlot->SetVerticalAlignment(VAlign_Fill);

		mSlotWidgetArray.Add(SlotWidget);

		SlotWidget->SetItem(InventoryComponent->GetItem(i));
	}

	InventoryComponent->AddItemChangeCallback<UInventoryWidget>(this, &UInventoryWidget::ItemChange);
	InventoryComponent->AddItemCountChangeCallback<UInventoryWidget>(this, &UInventoryWidget::ItemCountChange);
}

void UInventoryWidget::CloseInventory()
{
	// 창을 닫으면 입력 모드도 바뀌어야 하므로 직접 숨기지 않고 컨트롤러에 요청한다.
	if (AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>())
		PlayerController->SetInventoryOpen(false);
}

void UInventoryWidget::ItemChange(UItemObject* Item, int32 Index)
{
	mSlotWidgetArray[Index]->SetItem(Item);
}

void UInventoryWidget::ItemCountChange(int32 Count, int32 Index)
{
	mSlotWidgetArray[Index]->SetItemCount(Count);
}
