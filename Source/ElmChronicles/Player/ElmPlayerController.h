// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../GameInfo.h"
#include "GameFramework/PlayerController.h"
#include "ElmPlayerController.generated.h"

/**
 *
 */
UCLASS()
class ELMCHRONICLES_API AElmPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AElmPlayerController();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPlayerHUDWidget> mPlayerHUDWidgetClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UPlayerHUDWidget> mPlayerHUDWidget;

	// 주변 상호작용 대상 찾기, 안내 문구, F 키. 대상이 NPC인지 포탈인지는 이 컴포넌트도 모른다.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UInteractionComponent> mInteraction;

	// NPC와의 대화 진행. 대화창, 대화 중 입력 교체, 다음 줄 넘기기.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UDialogueComponent> mDialogue;

	FString mPlayerName;

public:
	class UPlayerHUDWidget* GetPlayerHUDWidget() const { return mPlayerHUDWidget; }
	class UInteractionComponent* GetInteraction() const { return mInteraction; }
	class UDialogueComponent* GetDialogue() const { return mDialogue; }
	const FString& GetPlayerName() const { return mPlayerName; }

public:
	void SetPlayerName(const FString& Name) { mPlayerName = Name; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

public:
	void ToggleInventory();
	void SetInventoryOpen(bool Open);
	void ToggleMenu();
	// 메뉴를 열면 게임을 멈추고, 닫으면 다시 움직인다. 메뉴의 "계속하기" / "귀환" 버튼도 이것으로 닫는다.
	void SetMenuOpen(bool Open);

private:
	void RefreshInputMode();
};
