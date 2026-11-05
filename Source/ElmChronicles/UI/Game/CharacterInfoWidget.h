// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "CharacterInfoWidget.generated.h"

struct FOnAttributeChangeData;

// C 키로 여는 캐릭터 창. "장비" 탭(장비칸 3개 + 장비가 바꾸는 능력치)과 "능력치" 탭을 버튼으로 바꾼다.
// 가운데 캐릭터 미리보기(PreviewArea)는 3단계에서 채운다. 열고 닫기는 컨트롤러(SetCharacterInfoOpen)가 한다.
UCLASS()
class ELMCHRONICLES_API UCharacterInfoWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UCharacterInfoWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// WBP_CharacterInfo에 같은 이름의 위젯이 있어야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mCloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mEquipTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mStatTabButton;

	// 0번 자식 = 장비 탭, 1번 자식 = 능력치 탭. WBP에서 자식 순서를 이대로 둔다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> mTabSwitcher;

	// 장비칸. WBP_Slot을 놓고 이름을 맞춘다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlotWidget> mWeaponSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlotWidget> mArmorSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlotWidget> mNecklaceSlot;

	// 장비 탭 아래 : 장비가 바꾸는 능력치 네 개
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mEquipStatText;

	// 능력치 탭 : 레벨, 경험치, 골드, HP / MP, 공격 속도, 이동 속도, 사거리
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mDetailStatText;

	// 장비칸 위젯을 EEquipSlot 순서로 모아 둔다. 변경 알림의 칸 번호로 바로 찾는다.
	UPROPERTY()
	TArray<TObjectPtr<class USlotWidget>> mEquipSlotArray;

	// 구독한 대상. 위젯이 사라질 때 구독을 끊으려고 들고 있는다.
	TWeakObjectPtr<class UInventoryComponent> mInventoryComponent;
	TWeakObjectPtr<class UAbilitySystemComponent> mASC;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 버튼의 OnClicked는 Dynamic 델리게이트라서 UFUNCTION 함수만 연결할 수 있다.
	UFUNCTION()	void OnCloseClicked();
	UFUNCTION()	void OnEquipTabClicked();
	UFUNCTION()	void OnStatTabClicked();
	void SelectTab(int32 TabIndex);
	void EquipChange(class UItemObject* Item, int32 Index);
	void OnAttributeChanged(const FOnAttributeChangeData& Data);
	void RefreshStats();
};
