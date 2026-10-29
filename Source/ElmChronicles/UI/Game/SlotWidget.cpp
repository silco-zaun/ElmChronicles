// Fill out your copyright notice in the Description page of Project Settings.


#include "SlotWidget.h"
#include "IconWidget.h"
#include "../../Item/ItemObject.h"
#include "IconDragDropOperation.h"
#include "../../Player/InventoryComponent.h"
#include "Engine/Texture2D.h"

USlotWidget::USlotWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void USlotWidget::SetInventoryComponent(UInventoryComponent* Inventory)
{
	mInventoryComponent = Inventory;
}

UInventoryComponent* USlotWidget::GetInventoryComponent() const
{
	return mInventoryComponent.Get();
}

void USlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	mIconWidget->SetParentSlot(this);
}

bool USlotWidget::NativeOnDrop(const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UIconDragDropOperation* DragIcon = Cast<UIconDragDropOperation>(InOperation);

	if (DragIcon && DragIcon->mDragStartSlot->mIndex != mIndex)
	{
		UE_LOG(LogTestDebug, Warning, TEXT("NativeOnDrop : Drag %d to %d"), DragIcon->mDragStartSlot->mIndex, mIndex);

		UInventoryComponent* InventoryComponent = GetInventoryComponent();

		if (InventoryComponent)
		{
			InventoryComponent->SwapItem(DragIcon->mDragStartSlot->mIndex, mIndex);
		}
	}

	return false;

}

void USlotWidget::SetItem(UItemObject* Item)
{
	if (Item)
	{
		mIconWidget->SetIconImage(Item->GetItemInfo().Icon.LoadSynchronous());
		mIconWidget->SetVisibility(ESlateVisibility::Visible);

		if (Item->GetItemInfo().UILayerType == EItemUILayerType::Layered)
		{
			mIconWidget->EnableCount(true);
			mIconWidget->SetCount(Item->GetItemCount());
		}
		else
		{
			mIconWidget->EnableCount(false);
		}
	}
	else
	{
		mIconWidget->SetIconImage(nullptr);
		mIconWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USlotWidget::SetItemCount(int32 Count)
{
	mIconWidget->SetCount(Count);
}
