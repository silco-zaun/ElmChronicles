// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UInventoryWidget : public UElmBaseWidget
{
	GENERATED_BODY()
	
public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// 위젯 블루프린트에서 배치한 위젯이 변수여부 체크가 되어있을 경우
	// UPROPERTY의 meta에 BindWidget을 이용해서 배치한 위젯의 객체를
	// 얻어올 수 있다. 변수 이름은 반드시 위젯 블루프린트에서 작성한 이름과
	// 동일해야 한다.
	UPROPERTY(VisibleAnywhere) TObjectPtr<UWidget> mTitleBar;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> mCloseButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<class UUniformGridPanel> mSlotGrid;
	// 동적으로 생성할 슬롯 위젯 클래스. WBP_Inventory의 Class Defaults에서 WBP_Slot을 지정한다.
	UPROPERTY(EditAnywhere, Category = "Inventory") TSubclassOf<class USlotWidget> mSlotWidgetClass;
	UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<class USlotWidget>> mSlotWidgetArray;
	// 한 줄에 배치할 슬롯 개수
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1)) int32 mColumnCount = 6;

	TWeakObjectPtr<class UInventoryComponent> mInventoryComponent;
	bool mIsDragging = false;

	// Drag를 시작한 마우스 위치.
	FVector2D mDragStartMousePos;

	// Drag를 시작했을 때 위젯의 위치.
	FVector2D mDragStartWidgetPos;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void InitInventory(class UInventoryComponent* InventoryComponent);
	UFUNCTION()	void CloseInventory();
	UFUNCTION()	void ItemChange(class UItemObject* Item, int32 Index);
	UFUNCTION()	void ItemCountChange(int32 Count, int32 Index);
};
