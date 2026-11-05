// Fill out your copyright notice in the Description page of Project Settings.


#include "IconWidget.h"
#include "IconDragDropOperation.h"
#include "PlayerHUDWidget.h"
#include "SlotWidget.h"
#include "../../Player/ElmPlayerController.h"
#include "../../Player/InventoryComponent.h"

UIconWidget::UIconWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

UPlayerHUDWidget* UIconWidget::GetPlayerHUD() const
{
	AElmPlayerController* PlayerController = GetOwningPlayer<AElmPlayerController>();

	return PlayerController ? PlayerController->GetPlayerHUDWidget() : nullptr;
}

void UIconWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

FReply UIconWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		UseSlotItem();
	else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 장비칸은 끌어서 옮기지 않는다(착용 / 해제는 우클릭 / 더블클릭). 드래그 감지 없이 여기서 끝내서
		// 클릭이 창 아래로 내려가지 않게 한다. 두 번째 누름은 NativeOnMouseButtonDoubleClick으로 간다.
		if (mParentSlot->GetType() == ESlotType::Equipment)
			return FReply::Handled();

		// Drag 시작 이벤트를 발생시킨다.
		FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent, this, EKeys::LeftMouseButton);

		return EventReply.NativeReply;
	}
	
	// Handled()로 처리를 하여 이 위젯이 마우스 클릭 이벤트를 처리했다는 의미가 된다.
	return FReply::Unhandled();
}

FReply UIconWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// 더블클릭의 두 번째 누름은 NativeOnMouseButtonDown이 아니라 여기로만 온다.
	// 왼쪽 : 첫 번째 누름은 위에서 드래그 감지로 받았으니, 두 번째 누름에서 사용한다.
	// 오른쪽 : 빠르게 우클릭을 두 번 하면 두 번째가 여기로 오므로, 우클릭 한 번으로 보고 사용한다.
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton ||
		InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		UseSlotItem();

		// 인벤토리 창(창 이동)이나 게임 화면으로 더블클릭이 넘어가지 않게 여기서 끝낸다.
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UIconWidget::NativeOnDragDetected(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// Drag를 하기 위한 Operation을 생성한다.
	UIconDragDropOperation* DragIcon = NewObject<UIconDragDropOperation>();

	DragIcon->mDragIcon = this;
	DragIcon->mDragStartSlot = mParentSlot;
	DragIcon->Pivot = EDragPivot::MouseDown;

	// Drag할 때 보여줄 Icon을 새로 생성하여 보여주게 한다.
	UIconWidget* DragRenderWidget = CreateWidget<UIconWidget>(GetWorld(),
		mDragIconClass);

	DragRenderWidget->mIconImage->SetBrushFromTexture(mIconTexture);
	// Alpha를 0.5로 주어서 반투명하게 출력되도록 한다.
	DragRenderWidget->mIconImage->SetBrushTintColor(FLinearColor(1.f, 1.f, 1.f, 0.5f));
	DragRenderWidget->EnableCount(false);

	DragIcon->DefaultDragVisual = DragRenderWidget;

	OutOperation = DragIcon;
}

void UIconWidget::NativeOnMouseEnter(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	UPlayerHUDWidget* PlayerHUD = GetPlayerHUD();

	if (!PlayerHUD)
		return;

	// 칸 종류를 넘겨서, 그 칸이 있는 창 옆에 정보창을 띄운다.
	PlayerHUD->ShowItemInfoWidget(true, mParentSlot->GetType());

	// 아이템 정보를 ItemInfoWidget에 전달해서 정보를 갱신한다.
	// 이 아이콘이 소속되어 있는 SlotWidget이 어떤 UI에 속한
	// 위젯인지 판단한다.
	switch (mParentSlot->GetType())
	{
	case ESlotType::Inventory:
		InventoryItemRender();
		break;
	case ESlotType::Equipment:
		EquipItemRender();
		break;
	case ESlotType::Skill:
		SkillInfoRender();
		break;
	case ESlotType::Quick:
		QuickSlotItemRender();
		break;
	}
}

void UIconWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (UPlayerHUDWidget* PlayerHUD = GetPlayerHUD())
		PlayerHUD->ShowItemInfoWidget(false);
}

void UIconWidget::SetIconImage(UTexture2D* Image)
{
	mIconTexture = Image;
	mIconImage->SetBrushFromTexture(Image);
}

void UIconWidget::SetCount(int32 Count)
{
	mCountText->SetText(FText::FromString(FString::FromInt(Count)));
}

void UIconWidget::EnableCount(bool Enable)
{
	if (Enable)
		mCountText->SetVisibility(ESlateVisibility::Visible);
	else
		mCountText->SetVisibility(ESlateVisibility::Collapsed);
}

void UIconWidget::UseSlotItem()
{
	switch (mParentSlot->GetType())
	{
	case ESlotType::Inventory:
		UseInventoryItem();
		break;
	case ESlotType::Equipment:
		UseEquipItem();
		break;
	case ESlotType::Skill:
		UseSkill();
		break;
	case ESlotType::Quick:
		UseQuickSlotItem();
		break;
	}
}

void UIconWidget::UseInventoryItem()
{
	UInventoryComponent* Inventory = mParentSlot->GetInventoryComponent();

	if (Inventory)
		Inventory->UseItem(mParentSlot->GetIndex());
}

void UIconWidget::UseEquipItem()
{
	UInventoryComponent* Inventory = mParentSlot->GetInventoryComponent();

	// 장비칸 슬롯의 번호는 EEquipSlot 값이다. (캐릭터 창이 그렇게 정해 둔다)
	if (Inventory)
		Inventory->UnequipItem(static_cast<EEquipSlot>(mParentSlot->GetIndex()));
}

void UIconWidget::UseSkill()
{
}

void UIconWidget::UseQuickSlotItem()
{
}

void UIconWidget::InventoryItemRender()
{
	UInventoryComponent* Inventory = mParentSlot->GetInventoryComponent();
	UPlayerHUDWidget* PlayerHUD = GetPlayerHUD();

	if (!Inventory || !PlayerHUD)
		return;

	UItemObject* Item = Inventory->GetItem(mParentSlot->GetIndex());

	if (Item)
		PlayerHUD->SetItemInfo(Item);
}

void UIconWidget::EquipItemRender()
{
	UInventoryComponent* Inventory = mParentSlot->GetInventoryComponent();
	UPlayerHUDWidget* PlayerHUD = GetPlayerHUD();

	if (!Inventory || !PlayerHUD)
		return;

	UItemObject* Item = Inventory->GetEquipItem(static_cast<EEquipSlot>(mParentSlot->GetIndex()));

	if (Item)
		PlayerHUD->SetItemInfo(Item);
}

void UIconWidget::QuickSlotItemRender()
{

}

void UIconWidget::SkillInfoRender()
{

}
