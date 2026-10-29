// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../UIInfo.h"
#include "Blueprint/UserWidget.h"
#include "SlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API USlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USlotWidget(const FObjectInitializer& ObjectInitializer);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UIconWidget> mIconWidget;

	uint32 mIndex = 0;
	ESlotType mType = ESlotType::Inventory;
	// UI는 게임 오브젝트를 소유하지 않으므로 약한 참조로 둔다.
	TWeakObjectPtr<class UInventoryComponent> mInventoryComponent;

public:
	void SetSlotIndex(uint32 Index)	{ mIndex = Index; }
	void SetSlotType(ESlotType Type) { mType = Type; }
	void SetInventoryComponent(class UInventoryComponent* Inventory);
	class UInventoryComponent* GetInventoryComponent() const;

public:
	ESlotType GetType() const { return mType; }
	uint32 GetIndex() const { return mIndex; }

protected:
	virtual void NativeOnInitialized();
	virtual bool NativeOnDrop(const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

public:
	void SetItem(class UItemObject* Item);
	void SetItemCount(int32 Count);
};
