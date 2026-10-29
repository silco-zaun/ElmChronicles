// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ElmBaseWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ELMCHRONICLES_API UPlayerHUDWidget : public UElmBaseWidget
{
	GENERATED_BODY()

public:
	UPlayerHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// 위젯 블루프린트에서 배치한 위젯이 변수여부 체크가 되어있을 경우
	// UPROPERTY의 meta에 BindWidget을 이용해서 배치한 위젯의 객체를
	// 얻어올 수 있다. 변수 이름은 반드시 위젯 블루프린트에서 작성한 이름과
	// 동일해야 한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> mInventoryWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UItemInfoWidget> mItemInfoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UStatusBarWidget> mStatusBarWidget;

	// 상호작용 안내 문구 ("[E] 대화하기"). 대상이 없으면 숨긴다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> mInteractText;

	// 화면 아래 대화창. 대화 중에만 보인다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UDialogueWidget> mDialogueWidget;

	// B 키 메뉴. 다른 창보다 위에 그려지도록 WBP_PlayerHUD Canvas의 마지막 자식으로 둔다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMenuWidget> mMenuWidget;

protected:
	virtual void NativeOnInitialized();

public:
	void SetPlayerName(const FString& Name);
	void SetItemInfo(class UItemObject* Item);
	void EnableItemInfoWidget(bool Enable = true);
	void ComputeItemInfoLocation(const FPointerEvent& InMouseEvent);
	void SetInventoryVisible(bool Visible);
	bool IsInventoryVisible() const;
	void SetMenuVisible(bool Visible);
	bool IsMenuVisible() const;
	// 커서가 필요한 창이 하나라도 열려 있는지
	bool IsAnyWindowOpen() const;
	// 상호작용 안내 문구를 띄운다. ActionText가 비어 있으면 숨긴다.
	void SetInteractPrompt(const FText& ActionText);
	// 대화창에 한 줄을 띄운다. 숨겨져 있었으면 보이게 한다.
	void SetDialogueLine(const FString& Speaker, const FString& Text);
	void SetDialogueVisible(bool Visible);
};
