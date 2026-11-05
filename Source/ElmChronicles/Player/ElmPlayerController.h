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

	// 받은 퀘스트, 처치 세기, 완료와 보상.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UQuestComponent> mQuest;

	FString mPlayerName;

public:
	class UPlayerHUDWidget* GetPlayerHUDWidget() const { return mPlayerHUDWidget; }
	class UInteractionComponent* GetInteraction() const { return mInteraction; }
	class UDialogueComponent* GetDialogue() const { return mDialogue; }
	class UQuestComponent* GetQuest() const { return mQuest; }
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
	void ToggleQuest();
	void SetQuestOpen(bool Open);
	void ToggleCharacterInfo();
	void SetCharacterInfoOpen(bool Open);
	void ToggleMenu();
	// 메뉴를 열면 게임을 멈추고, 닫으면 다시 움직인다. 메뉴의 "계속하기" / "귀환" 버튼도 이것으로 닫는다.
	void SetMenuOpen(bool Open);
	// 확인 상자를 열고 닫는다. 창 위에서든 창 없이든, 열려 있는 동안 게임을 멈추고 커서를 띄운다.
	// 확인 상자의 "예" / "아니요"도 CloseConfirm으로 닫는다.
	void OpenConfirm(const FText& Message, FSimpleDelegate OnYes);
	void CloseConfirm();
	// "게임을 종료할까요?"를 묻는다. 메뉴의 게임 종료와 Alt+F4가 같이 쓴다.
	void RequestQuit();

private:
	void RefreshInputMode();
	// Alt+F4 / 창의 X. false를 돌려주면 창이 닫히지 않는다. (UGameViewportClient::OnWindowCloseRequested)
	bool OnWindowCloseRequested();
	// 종료 확인 상자에서 "예"를 누르면 불린다.
	void OnQuitConfirmed();
};
