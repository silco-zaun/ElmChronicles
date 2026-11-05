// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../GameInfo.h"
#include "../UIInfo.h"
#include "Blueprint/UserWidget.h"
#include "IconWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UIconWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UIconWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> mIconImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> mCountText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UIconWidget> mDragIconClass;
	
	UPROPERTY()	class USlotWidget* mParentSlot = nullptr;
	UPROPERTY() UTexture2D* mIconTexture = nullptr;

public:
	void SetParentSlot(class USlotWidget* ParentSlot)
	{
		mParentSlot = ParentSlot;
	}
private:
	class UPlayerHUDWidget* GetPlayerHUD() const;

protected:
	virtual void NativeOnInitialized();
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent);

public:
	void SetIconImage(UTexture2D* Image);
	void SetCount(int32 Count);
	void EnableCount(bool Enable);

private:
	// 이 아이콘이 속한 칸 종류에 맞게 "사용"한다. 우클릭과 더블클릭이 같이 부른다.
	void UseSlotItem();
	void UseInventoryItem();
	// 장비칸의 "사용"은 벗기다. 가방이 가득 차 있으면 아무 일도 일어나지 않는다.
	void UseEquipItem();
	void UseSkill();
	void UseQuickSlotItem();
	void InventoryItemRender();
	void EquipItemRender();
	void QuickSlotItemRender();
	void SkillInfoRender();
};
