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

	FString mPlayerName;
	// 지금 E를 누르면 상호작용할 대상. 안내 문구도 이 대상 기준으로 띄운다.
	// 대상이 사라지면 자동으로 nullptr이 되도록 약한 포인터로 들고 있는다.
	TWeakObjectPtr<AActor> mInteractTarget;
	FTimerHandle mInteractTimer;

	// 플레이어 중심에서 이 반지름(cm) 안에 캡슐이 걸친 대상을 찾는다.
	static constexpr float InteractRadius = 200.f;
	// 대상 검색 주기(초). 매 프레임 할 필요는 없다.
	static constexpr float InteractScanInterval = 0.1f;

public:
	class UPlayerHUDWidget* GetPlayerHUDWidget() const { return mPlayerHUDWidget; }
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
	// 주변의 상호작용 대상을 찾아 mInteractTarget과 안내 문구를 갱신한다. (타이머로 0.1초마다)
	void UpdateInteractTarget();
	void InteractKey();
};
